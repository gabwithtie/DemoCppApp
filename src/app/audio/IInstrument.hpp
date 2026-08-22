// src/app/audio/IInstrument.hpp
#pragma once
#include "midi/MidiInterface.hpp"
#include <cstddef>

namespace gsr::audio {

class IInstrument {
public:
    virtual ~IInstrument() = default;
    virtual void ProcessMidiEvent(const midi::MidiMessage& msg) = 0;
    virtual void RenderAudioBlock(float* output_buffer, size_t num_frames, double sample_rate) = 0;
};

} // namespace gsr::audio