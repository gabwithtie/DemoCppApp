#pragma once

#include "PngWriter.h"

#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace picsel {

    // -------------------------------------------------------------------------
    // PngCompositor
    //
    // Reads a list of PNG files (written by PngWriter — 8-bit RGBA, uncompressed
    // zlib, no interlacing, filter-none) and composites them in order from
    // bottom (index 0) to top (last index) using standard alpha-over blending.
    // The result is written to output_path using PngWriter's own encoding path.
    //
    // Important constraints (matching PngWriter's output format):
    //   - 8-bit RGBA only
    //   - Uncompressed zlib deflate blocks only (CM=8, no Huffman/LZ77)
    //   - PNG filter byte 0x00 (None) per scanline
    //   - No interlacing
    //
    // If a layer file is missing or unreadable it is silently skipped.
    // If no layers produce valid pixel data, the output is a blank transparent PNG.
    // -------------------------------------------------------------------------
    class PngCompositor {
    public:
        // Composite layer_paths (bottom to top) into output_path.
        // width/height are the canvas dimensions — layers that don't match are skipped.
        static bool FlattenLayers(
            const std::vector<std::filesystem::path>& layer_paths,
            const std::filesystem::path& output_path,
            int width,
            int height)
        {
            // Start with a fully transparent canvas
            const int    pixel_count = width * height;
            std::vector<uint8_t> composite(pixel_count * 4, 0x00);

            for (const auto& path : layer_paths) {
                std::vector<uint8_t> layer_pixels;
                int lw = 0, lh = 0;

                if (!ReadRGBA(path, layer_pixels, lw, lh)) {
                    std::cerr << "[PngCompositor] Skipping unreadable layer: " << path << "\n";
                    continue;
                }
                if (lw != width || lh != height) {
                    std::cerr << "[PngCompositor] Dimension mismatch, skipping: " << path
                              << " (" << lw << "x" << lh << " vs " << width << "x" << height << ")\n";
                    continue;
                }

                AlphaOver(composite.data(), layer_pixels.data(), pixel_count);
            }

            return WriteComposite(output_path, composite.data(), width, height);
        }

    private:
        // -----------------------------------------------------------------
        // Alpha-over blending (Porter-Duff "over")
        // dst is modified in-place; src is composited on top.
        // Both buffers are width*height*4 bytes, RGBA.
        // -----------------------------------------------------------------
        static void AlphaOver(uint8_t* dst, const uint8_t* src, int pixel_count) {
            for (int i = 0; i < pixel_count; ++i) {
                const uint8_t* s = src + i * 4;
                uint8_t*       d = dst + i * 4;

                // Work in [0,1] float space for correctness, then clamp back.
                float sa = s[3] / 255.0f;
                float da = d[3] / 255.0f;

                float out_a = sa + da * (1.0f - sa);
                if (out_a < 1e-6f) {
                    d[0] = d[1] = d[2] = d[3] = 0;
                    continue;
                }

                for (int c = 0; c < 3; ++c) {
                    float sc = s[c] / 255.0f;
                    float dc = d[c] / 255.0f;
                    float out_c = (sc * sa + dc * da * (1.0f - sa)) / out_a;
                    d[c] = static_cast<uint8_t>(std::min(out_c * 255.0f + 0.5f, 255.0f));
                }
                d[3] = static_cast<uint8_t>(std::min(out_a * 255.0f + 0.5f, 255.0f));
            }
        }

        // -----------------------------------------------------------------
        // Minimal PNG reader — handles exactly the format PngWriter produces:
        //   8-bit RGBA, uncompressed zlib stored blocks, filter-none scanlines.
        //
        // Returns false if the file can't be opened, has an unexpected signature,
        // wrong color type/depth, or any chunk is unreadable.
        // -----------------------------------------------------------------
        static bool ReadRGBA(
            const std::filesystem::path& path,
            std::vector<uint8_t>& out_pixels,
            int& out_width,
            int& out_height)
        {
            std::ifstream f(path, std::ios::binary);
            if (!f.is_open()) return false;

            // --- PNG signature ---
            uint8_t sig[8];
            if (!ReadBytes(f, sig, 8)) return false;
            const uint8_t expected_sig[8] = { 0x89,'P','N','G','\r','\n',0x1A,'\n' };
            if (std::memcmp(sig, expected_sig, 8) != 0) return false;

            int width = 0, height = 0;
            std::vector<uint8_t> idat_payload; // accumulated raw zlib stream bytes

            // --- Chunk loop ---
            while (f.good()) {
                uint8_t len_buf[4];
                if (!ReadBytes(f, len_buf, 4)) break;
                uint32_t chunk_len = ReadBE32(len_buf);

                char type[5] = {};
                if (!ReadBytes(f, reinterpret_cast<uint8_t*>(type), 4)) break;

                std::vector<uint8_t> chunk_data(chunk_len);
                if (chunk_len > 0 && !ReadBytes(f, chunk_data.data(), chunk_len)) break;

                // CRC — read and discard (we trust our own writer)
                uint8_t crc_buf[4];
                ReadBytes(f, crc_buf, 4);

                if (std::strcmp(type, "IHDR") == 0) {
                    if (chunk_len < 13) return false;
                    width  = static_cast<int>(ReadBE32(&chunk_data[0]));
                    height = static_cast<int>(ReadBE32(&chunk_data[4]));
                    uint8_t bit_depth  = chunk_data[8];
                    uint8_t color_type = chunk_data[9];
                    uint8_t interlace  = chunk_data[12];
                    if (bit_depth != 8 || color_type != 6 || interlace != 0) {
                        std::cerr << "[PngCompositor] Unsupported PNG format in " << path << "\n";
                        return false;
                    }
                }
                else if (std::strcmp(type, "IDAT") == 0) {
                    // Accumulate all IDAT chunks (there's only one from PngWriter,
                    // but the spec allows multiple — handle both cases).
                    idat_payload.insert(idat_payload.end(), chunk_data.begin(), chunk_data.end());
                }
                else if (std::strcmp(type, "IEND") == 0) {
                    break;
                }
                // All other chunk types (tEXt, gAMA, etc.) are ignored.
            }

            if (width <= 0 || height <= 0 || idat_payload.empty()) return false;

            // --- Decode zlib uncompressed stream ---
            // Layout: [0x78][0x01] then one or more stored blocks, then Adler-32
            std::vector<uint8_t> raw_scanlines;
            if (!DecodeZlibStored(idat_payload, raw_scanlines)) return false;

            // --- Reconstruct pixels from filter-none scanlines ---
            uint32_t stride    = static_cast<uint32_t>(width) * 4;
            uint32_t expected  = static_cast<uint32_t>(height) * (1 + stride);
            if (raw_scanlines.size() < expected) return false;

            out_width  = width;
            out_height = height;
            out_pixels.resize(static_cast<size_t>(width) * height * 4);

            for (int y = 0; y < height; ++y) {
                uint32_t row_offset = static_cast<uint32_t>(y) * (1 + stride);
                // uint8_t filter_byte = raw_scanlines[row_offset]; // always 0x00 from PngWriter
                std::memcpy(out_pixels.data() + y * stride,
                            raw_scanlines.data() + row_offset + 1,
                            stride);
            }
            return true;
        }

        // -----------------------------------------------------------------
        // Decodes the uncompressed (stored) zlib stream PngWriter generates.
        // Format: 2-byte zlib header | N deflate stored blocks | 4-byte Adler-32
        //
        // A stored deflate block is:
        //   [BFINAL|BTYPE=00] [LEN lo] [LEN hi] [~LEN lo] [~LEN hi] [LEN bytes]
        // -----------------------------------------------------------------
        static bool DecodeZlibStored(
            const std::vector<uint8_t>& zlib,
            std::vector<uint8_t>& out_raw)
        {
            if (zlib.size() < 6) return false; // 2 hdr + 5 min block + 4 adler

            // Validate zlib header (CM=8, CINFO=7 → 0x78; FCHECK → 0x01 or 0x9C etc.)
            if ((zlib[0] & 0x0F) != 8) return false; // CM must be deflate

            size_t pos = 2; // skip 2-byte zlib header
            out_raw.clear();

            while (pos + 5 <= zlib.size()) {
                uint8_t  bfinal_btype = zlib[pos];
                uint8_t  btype        = (bfinal_btype >> 1) & 0x03;
                bool     is_last      = (bfinal_btype & 0x01) != 0;
                pos++;

                if (btype != 0x00) {
                    // Compressed blocks — not produced by PngWriter, not supported here.
                    std::cerr << "[PngCompositor] Compressed deflate block encountered — "
                                 "only stored (uncompressed) blocks supported.\n";
                    return false;
                }

                if (pos + 4 > zlib.size()) return false;
                uint16_t block_len  = static_cast<uint16_t>(zlib[pos])
                                    | (static_cast<uint16_t>(zlib[pos + 1]) << 8);
                uint16_t nlen       = static_cast<uint16_t>(zlib[pos + 2])
                                    | (static_cast<uint16_t>(zlib[pos + 3]) << 8);
                pos += 4;

                if (static_cast<uint16_t>(~block_len) != nlen) return false; // integrity check

                if (pos + block_len > zlib.size()) return false;
                out_raw.insert(out_raw.end(),
                               zlib.begin() + pos,
                               zlib.begin() + pos + block_len);
                pos += block_len;

                if (is_last) break;
            }
            // Remaining 4 bytes are the Adler-32 checksum — we skip verification
            // since we're reading our own output and speed matters more here.
            return !out_raw.empty();
        }

        // -----------------------------------------------------------------
        // Encode composite pixel buffer back to PNG using the same uncompressed
        // zlib path as PngWriter. Shares all the same private helpers.
        // -----------------------------------------------------------------
        static bool WriteComposite(
            const std::filesystem::path& output_path,
            const uint8_t* pixels, // width*height*4, RGBA, no filter bytes
            int width, int height)
        {
            std::ofstream file(output_path, std::ios::binary);
            if (!file.is_open()) return false;

            // PNG signature
            const uint8_t sig[8] = { 0x89,'P','N','G','\r','\n',0x1A,'\n' };
            file.write(reinterpret_cast<const char*>(sig), 8);

            // IHDR
            uint8_t ihdr[13] = {};
            WriteBE32(&ihdr[0], static_cast<uint32_t>(width));
            WriteBE32(&ihdr[4], static_cast<uint32_t>(height));
            ihdr[8] = 8; ihdr[9] = 6; // 8-bit RGBA
            WriteChunk(file, "IHDR", ihdr, 13);

            // Build raw scanlines (filter byte 0x00 per row)
            uint32_t stride       = static_cast<uint32_t>(width) * 4;
            uint32_t raw_size     = static_cast<uint32_t>(height) * (1 + stride);
            std::vector<uint8_t> raw(raw_size);

            for (int y = 0; y < height; ++y) {
                uint32_t row = static_cast<uint32_t>(y) * (1 + stride);
                raw[row] = 0x00; // filter none
                std::memcpy(raw.data() + row + 1,
                            pixels + y * stride,
                            stride);
            }

            // Wrap in uncompressed zlib
            std::vector<uint8_t> zlib;
            zlib.push_back(0x78);
            zlib.push_back(0x01);

            uint32_t left = raw_size, offset = 0;
            while (left > 0) {
                uint16_t bsz     = (left > 65535) ? 65535 : static_cast<uint16_t>(left);
                bool     is_last = (bsz == left);
                zlib.push_back(is_last ? 0x01 : 0x00);
                zlib.push_back(bsz & 0xFF);
                zlib.push_back((bsz >> 8) & 0xFF);
                uint16_t nbsz = ~bsz;
                zlib.push_back(nbsz & 0xFF);
                zlib.push_back((nbsz >> 8) & 0xFF);
                zlib.insert(zlib.end(), raw.begin() + offset, raw.begin() + offset + bsz);
                offset += bsz;
                left   -= bsz;
            }

            uint8_t adler_buf[4];
            WriteBE32(adler_buf, ComputeAdler32(raw.data(), raw_size));
            zlib.insert(zlib.end(), adler_buf, adler_buf + 4);

            WriteChunk(file, "IDAT", zlib.data(), static_cast<uint32_t>(zlib.size()));
            WriteChunk(file, "IEND", nullptr, 0);
            file.close();
            return true;
        }

        // -----------------------------------------------------------------
        // Low-level helpers (mirrors of PngWriter's private statics)
        // -----------------------------------------------------------------
        static bool ReadBytes(std::ifstream& f, uint8_t* buf, size_t n) {
            return static_cast<bool>(f.read(reinterpret_cast<char*>(buf), n));
        }

        static uint32_t ReadBE32(const uint8_t* b) {
            return (static_cast<uint32_t>(b[0]) << 24)
                 | (static_cast<uint32_t>(b[1]) << 16)
                 | (static_cast<uint32_t>(b[2]) <<  8)
                 |  static_cast<uint32_t>(b[3]);
        }

        static void WriteBE32(uint8_t* buf, uint32_t v) {
            buf[0] = (v >> 24) & 0xFF; buf[1] = (v >> 16) & 0xFF;
            buf[2] = (v >>  8) & 0xFF; buf[3] =  v        & 0xFF;
        }

        static void UpdateCRC32(uint32_t& crc, const uint8_t* data, uint32_t len) {
            for (uint32_t i = 0; i < len; ++i) {
                crc ^= data[i];
                for (int j = 0; j < 8; ++j)
                    crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320) : (crc >> 1);
            }
        }

        static void WriteChunk(std::ofstream& s, const char* type,
                               const uint8_t* data, uint32_t len) {
            uint8_t lb[4]; WriteBE32(lb, len);
            s.write(reinterpret_cast<const char*>(lb), 4);
            s.write(type, 4);
            if (data && len) s.write(reinterpret_cast<const char*>(data), len);

            uint32_t crc = 0xFFFFFFFF;
            UpdateCRC32(crc, reinterpret_cast<const uint8_t*>(type), 4);
            if (data && len) UpdateCRC32(crc, data, len);
            crc ^= 0xFFFFFFFF;

            uint8_t cb[4]; WriteBE32(cb, crc);
            s.write(reinterpret_cast<const char*>(cb), 4);
        }

        static uint32_t ComputeAdler32(const uint8_t* data, uint32_t len) {
            uint32_t s1 = 1, s2 = 0;
            for (uint32_t i = 0; i < len; ++i) {
                s1 = (s1 + data[i]) % 65521;
                s2 = (s2 + s1)      % 65521;
            }
            return (s2 << 16) | s1;
        }
    };

} // namespace picsel
