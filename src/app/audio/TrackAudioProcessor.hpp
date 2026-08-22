// src/app/audio/TrackAudioProcessor.hpp
#pragma once
#include "IInstrument.hpp"
#include "midi/MidiInterface.hpp"
#include <memory>
#include <vector>
#include <iostream>

namespace gsr::audio {

class TrackAudioProcessor {
public:
    TrackAudioProcessor() = default;

    void SetInstrument(std::unique_ptr<IInstrument> instrument) {
        m_instrument = std::move(instrument);
        m_warned_missing_instrument = false; // Reset warning whenever an instrument changes
    }

    [[nodiscard]] bool HasInstrument() const {
        return m_instrument != nullptr;
    }

    midi::MidiInterface& GetMidiInterface() { return m_midi_interface; }

    void ProcessAndRender(float* track_output, size_t num_frames, double sample_rate) {
        // 1. Flush MIDI queued specifically for this track
        std::vector<midi::MidiMessage> pending_messages;
        m_midi_interface.FlushPendingMessages(pending_messages);

        // 2. Dispatch MIDI to active instrument and render buffer
        if (m_instrument) {
            for (const auto& msg : pending_messages) {
                m_instrument->ProcessMidiEvent(msg);
            }
            m_instrument->RenderAudioBlock(track_output, num_frames, sample_rate);
        } else {
            // Fill audio output with silence
            std::fill_n(track_output, num_frames * 2, 0.0f);

            // Log once if incoming MIDI events arrive while no instrument is bound
            if (!pending_messages.empty() && !m_warned_missing_instrument) {
                std::cerr << "[TrackAudioProcessor Warning] Discarded " << pending_messages.size() 
                          << " MIDI message(s): No instrument assigned to processor.\n";
                m_warned_missing_instrument = true;
            }
        }
    }

private:
    std::unique_ptr<IInstrument> m_instrument;
    midi::MidiInterface m_midi_interface;
    bool m_warned_missing_instrument{false};
};

} // namespace gsr::audio