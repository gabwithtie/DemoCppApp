#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "Track.hpp"

namespace Model {

enum class KeyType { Major, Minor };

struct KeySignature {
    int8_t accidentals{0}; // -7 (Flats) to +7 (Sharps)
    KeyType type{KeyType::Major};
};

struct KeySignatureChange {
    uint32_t bar_index{0}; // Measure index where key changes
    KeySignature key;
};

struct TimeSignature {
    uint8_t numerator{4};
    uint8_t denominator{4};
};

struct TimeSignatureChange {
    uint32_t bar_index{0}; // Measure index where meter changes
    TimeSignature time_sig;
};

struct TempoPoint {
    uint64_t tick{0};   // Position in ticks for smooth automation
    double bpm{120.0};  // Beats per minute
};

struct Project {
    std::string title{"Untitled Project"};
    uint32_t ppq{960};  // Resolution: Pulses Per Quarter note

    // Master timelines (must remain sorted chronologically)
    std::vector<TempoPoint> tempo_map{ {0, 120.0} };
    std::vector<TimeSignatureChange> time_sig_map{ {0, {4, 4}} };
    std::vector<KeySignatureChange> key_sig_map{ {0, {0, KeyType::Major}} };

    std::vector<Track> tracks;
};

} // namespace Model