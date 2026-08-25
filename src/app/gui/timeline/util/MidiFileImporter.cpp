#include "MidiFileImporter.hpp"

#include <algorithm>
#include <fstream>

namespace gsr::gui {
namespace {

bool ReadBytes(std::ifstream& file, char* data, std::streamsize size) {
    return file.read(data, size).good();
}

uint32_t ReadBigEndian32(const char* data) {
    return (static_cast<uint32_t>(static_cast<unsigned char>(data[0])) << 24) |
           (static_cast<uint32_t>(static_cast<unsigned char>(data[1])) << 16) |
           (static_cast<uint32_t>(static_cast<unsigned char>(data[2])) << 8) |
           static_cast<uint32_t>(static_cast<unsigned char>(data[3]));
}

uint16_t ReadBigEndian16(const char* data) {
    return static_cast<uint16_t>((static_cast<uint16_t>(static_cast<unsigned char>(data[0])) << 8) |
                                 static_cast<uint16_t>(static_cast<unsigned char>(data[1])));
}

bool ReadVariableLength(const std::vector<unsigned char>& data, size_t& offset, uint32_t& value) {
    value = 0;
    for (int i = 0; i < 4; ++i) {
        if (offset >= data.size()) return false;
        const unsigned char byte = data[offset++];
        value = (value << 7) | (byte & 0x7f);
        if ((byte & 0x80) == 0) return true;
    }
    return false;
}

} // namespace

bool ImportMidiFile(const std::string& path, ImportedMidi& imported) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    char header[8];
    if (!ReadBytes(file, header, sizeof(header)) || std::string(header, 4) != "MThd") return false;
    const uint32_t header_size = ReadBigEndian32(header + 4);
    if (header_size < 6) return false;

    std::vector<char> header_data(header_size);
    if (!ReadBytes(file, header_data.data(), static_cast<std::streamsize>(header_data.size()))) return false;

    const uint16_t format = ReadBigEndian16(header_data.data());
    const uint16_t track_count = ReadBigEndian16(header_data.data() + 2);
    const uint16_t division = ReadBigEndian16(header_data.data() + 4);
    if (format > 1 || track_count == 0 || (division & 0x8000) != 0 || division == 0) return false;

    imported.ppq = division;
    imported.notes.clear();

    for (uint16_t track_index = 0; track_index < track_count; ++track_index) {
        char track_header[8];
        if (!ReadBytes(file, track_header, sizeof(track_header)) || std::string(track_header, 4) != "MTrk") return false;
        const uint32_t track_size = ReadBigEndian32(track_header + 4);
        std::vector<unsigned char> data(track_size);
        if (!ReadBytes(file, reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()))) return false;

        struct ActiveNote {
            uint64_t start_tick;
            unsigned char velocity;
        };
        std::vector<std::vector<ActiveNote>> active(16 * 128);
        size_t offset = 0;
        uint64_t tick = 0;
        unsigned char running_status = 0;

        while (offset < data.size()) {
            uint32_t delta = 0;
            if (!ReadVariableLength(data, offset, delta)) return false;
            tick += delta;
            if (offset >= data.size()) break;

            unsigned char status = data[offset++];
            if (status < 0x80) {
                if (running_status < 0x80 || running_status >= 0xf0) return false;
                --offset;
                status = running_status;
            } else if (status < 0xf0) {
                running_status = status;
            }

            if (status == 0xff) {
                if (offset >= data.size()) return false;
                ++offset;
                uint32_t meta_size = 0;
                if (!ReadVariableLength(data, offset, meta_size) || meta_size > data.size() - offset) return false;
                offset += meta_size;
                continue;
            }
            if (status == 0xf0 || status == 0xf7) {
                uint32_t sysex_size = 0;
                if (!ReadVariableLength(data, offset, sysex_size) || sysex_size > data.size() - offset) return false;
                offset += sysex_size;
                continue;
            }

            const unsigned char type = status & 0xf0;
            const unsigned char channel = status & 0x0f;
            const size_t message_size = (type == 0xc0 || type == 0xd0) ? 1 : 2;
            if (offset + message_size > data.size()) return false;

            const unsigned char pitch = data[offset++];
            const unsigned char velocity = message_size == 2 ? data[offset++] : 0;
            if (type == 0x90 && velocity != 0) {
                active[channel * 128 + pitch].push_back({tick, velocity});
            } else if (type == 0x80 || (type == 0x90 && velocity == 0)) {
                auto& starts = active[channel * 128 + pitch];
                if (!starts.empty()) {
                    const ActiveNote start = starts.back();
                    starts.pop_back();
                    if (tick > start.start_tick) {
                        imported.notes.push_back({pitch, start.start_tick, tick - start.start_tick, start.velocity, false, {}});
                    }
                }
            }
        }
    }

    std::sort(imported.notes.begin(), imported.notes.end(), [](const Model::Note& left, const Model::Note& right) {
        return left.start_tick < right.start_tick;
    });
    return !imported.notes.empty();
}

} // namespace gsr::gui