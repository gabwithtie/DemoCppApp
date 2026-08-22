#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace Model {

enum class ClipType {
    Standard,
    Repeat // Replays notes from the previous X bars
};

struct Note {
    uint8_t pitch{60};
    uint8_t velocity{100};
    uint64_t start_tick{0}; // Relative offset from clip start tick
    uint64_t duration{960};
    bool selected{false};
};

struct Clip {
    std::string name{"New Clip"};
    uint64_t start_tick{0};
    uint64_t duration{3840}; // Ticks (4 bars default @ 960 PPQ)
    std::vector<Note> notes;
    bool selected{false};
    ClipType type{ClipType::Standard};
};

struct Track {
    std::string name{"Track 1"};
    uint8_t midi_channel{0};
    bool muted{false};
    bool solo{false};
    float volume{1.0f};
    float pan{0.0f};

    std::vector<Clip> clips;
    std::vector<Note> notes;
};

} // namespace Model