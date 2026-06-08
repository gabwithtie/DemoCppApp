#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <cstdint>

namespace picsel {

    class PngWriter {
    public:
        static bool WriteEmptyRGBA(const std::filesystem::path& target_path, int width, int height) {
            std::ofstream file(target_path, std::ios::binary);
            if (!file.is_open()) return false;

            // 1. Standard 8-byte PNG Signature Header
            const uint8_t signature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };
            file.write(reinterpret_cast<const char*>(signature), 8);

            // 2. Build IHDR (Image Header) Chunk
            uint8_t ihdr_data[13];
            WriteBE32(&ihdr_data[0], width);
            WriteBE32(&ihdr_data[4], height);
            ihdr_data[8] = 8;  // Bit Depth: 8 bits per channel
            ihdr_data[9] = 6;  // Color Type: RGBA (Red, Green, Blue, Alpha)
            ihdr_data[10] = 0;  // Compression Method: Deflate
            ihdr_data[11] = 0;  // Filter Method: Adaptive
            ihdr_data[12] = 0;  // Interlace Method: No interlace
            WriteChunk(file, "IHDR", ihdr_data, 13);

            // 3. Build IDAT (Image Data) Chunk using raw uncompressed zlib deflate wrapper
            // Each horizontal scanline row requires 1 structural PNG Filter Byte prefix (0x00 = None)
            uint32_t stride = width * 4;
            uint32_t raw_data_size = height * (1 + stride);
            std::vector<uint8_t> raw_pixels(raw_data_size, 0x00);

            // Initialize pixels to a default semi-transparent layout checker color (optional)
            for (int y = 0; y < height; ++y) {
                uint32_t row_start = y * (1 + stride);
                raw_pixels[row_start] = 0x00; // Filter Type byte for this row

                for (int x = 0; x < width; ++x) {
                    uint32_t px = row_start + 1 + (x * 4);
                    // Initialize with a clean base color slate: Transparent gray-grid accent
                    raw_pixels[px + 0] = 45;  // R
                    raw_pixels[px + 1] = 45;  // G
                    raw_pixels[px + 2] = 45;  // B
                    raw_pixels[px + 3] = 255; // A (Fully Opaque Base)
                }
            }

            // Wrap raw payload inside standard uncompressed Zlib streams
            std::vector<uint8_t> zlib_payload;
            zlib_payload.push_back(0x78); // Zlib Header Byte 1 (CM=8 Deflate, CINFO=7 32K Window)
            zlib_payload.push_back(0x01); // Zlib Header Byte 2 (FCHECK value, No compression preset)

            // Deflate block configuration loops (Split into chunks <= 65535 bytes if needed)
            uint32_t bytes_left = raw_data_size;
            uint32_t src_offset = 0;
            while (bytes_left > 0) {
                uint16_t block_size = (bytes_left > 65535) ? 65535 : static_cast<uint16_t>(bytes_left);
                uint8_t bfinal_btype = (block_size == bytes_left) ? 0x01 : 0x00; // BFINAL bit set if last block

                zlib_payload.push_back(bfinal_btype);
                zlib_payload.push_back(block_size & 0xFF);
                zlib_payload.push_back((block_size >> 8) & 0xFF);

                uint16_t ones_complement = ~block_size;
                zlib_payload.push_back(ones_complement & 0xFF);
                zlib_payload.push_back((ones_complement >> 8) & 0xFF);

                zlib_payload.insert(zlib_payload.end(), raw_pixels.begin() + src_offset, raw_pixels.begin() + src_offset + block_size);
                bytes_left -= block_size;
                src_offset += block_size;
            }

            // Compute Adler-32 checksum across raw scanline bytes payload
            uint32_t adler = ComputeAdler32(raw_pixels.data(), raw_data_size);
            uint8_t adler_buf[4];
            WriteBE32(adler_buf, adler);
            zlib_payload.insert(zlib_payload.end(), adler_buf, adler_buf + 4);

            WriteChunk(file, "IDAT", zlib_payload.data(), static_cast<uint32_t>(zlib_payload.size()));

            // 4. Build IEND (Image Trailer) End Chunk
            WriteChunk(file, "IEND", nullptr, 0);

            file.close();
            return true;
        }

    private:
        static void WriteBE32(uint8_t* buffer, uint32_t value) {
            buffer[0] = (value >> 24) & 0xFF;
            buffer[1] = (value >> 16) & 0xFF;
            buffer[2] = (value >> 8) & 0xFF;
            buffer[3] = value & 0xFF;
        }

        static void UpdateCRC32(uint32_t& crc, const uint8_t* data, uint32_t length) {
            for (uint32_t i = 0; i < length; ++i) {
                uint8_t byte = data[i];
                crc ^= byte;
                for (int j = 0; j < 8; ++j) {
                    if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                    else crc >>= 1;
                }
            }
        }

        static void WriteChunk(std::ofstream& stream, const char* type, const uint8_t* data, uint32_t length) {
            uint8_t len_buf[4];
            WriteBE32(len_buf, length);
            stream.write(reinterpret_cast<const char*>(len_buf), 4);
            stream.write(type, 4);

            if (data && length > 0) {
                stream.write(reinterpret_cast<const char*>(data), length);
            }

            uint32_t crc = 0xFFFFFFFF;
            UpdateCRC32(crc, reinterpret_cast<const uint8_t*>(type), 4);
            if (data && length > 0) {
                UpdateCRC32(crc, data, length);
            }
            crc ^= 0xFFFFFFFF;

            uint8_t crc_buf[4];
            WriteBE32(crc_buf, crc);
            stream.write(reinterpret_cast<const char*>(crc_buf), 4);
        }

        static uint32_t ComputeAdler32(const uint8_t* data, uint32_t length) {
            uint32_t s1 = 1;
            uint32_t s2 = 0;
            for (uint32_t i = 0; i < length; ++i) {
                s1 = (s1 + data[i]) % 65521;
                s2 = (s2 + s1) % 65521;
            }
            return (s2 << 16) | s1;
        }
    };
}