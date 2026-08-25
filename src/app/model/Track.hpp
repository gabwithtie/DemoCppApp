#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../../gui/features/nodeeditor/GenericNodeEditor.hpp"

namespace Model {

enum class ClipType {
    Standard,
    Repeat // Replays notes from the previous X bars
};

struct Note {
    uint8_t pitch{60};
    uint64_t start_tick{0};
    uint64_t duration{480};
    uint8_t velocity{100};
    bool selected{false};

    // Sub-note resolution segment data for paint mode
    std::vector<float> paint_segments{};
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

    // Instrument Settings
    std::string instrument_type{"SoundFont Synthesizer"};
    std::string soundfont_path{"default/default.SF2"};
    int preset_index{0};
    float instrument_gain{1.0f};
    bool needs_reload{false}; // Signal flag for audio thread to rebuild instrument

    app::GraphData audio_graph;
    std::vector<Clip> clips;
    std::vector<Note> notes;
};

} // namespace Model