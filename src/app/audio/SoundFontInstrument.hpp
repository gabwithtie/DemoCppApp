#pragma once

#define TSF_IMPLEMENTATION

#include "IInstrument.hpp"
#include <tsf.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace gsr::audio {

class SoundFontInstrument : public IInstrument {
public:
    explicit SoundFontInstrument(const std::string& path, double sample_rate = 44100.0) {
        if (path.empty() || !std::filesystem::exists(path)) {
            std::cerr << "[SoundFont Error] Invalid path: " << path << "\n";
            return;
        }

        m_synth = tsf_load_filename(path.c_str());
        if (!m_synth) {
            std::cerr << "[SoundFont Error] Failed to parse SoundFont: " << path << "\n";
            return;
        }

        tsf_set_output(m_synth, TSF_STEREO_INTERLEAVED, static_cast<int>(sample_rate), 0.0f);
        m_is_loaded = true;
    }

    ~SoundFontInstrument() override {
        if (m_synth) {
            tsf_close(m_synth);
            m_synth = nullptr;
        }
    }

    [[nodiscard]] bool IsLoaded() const { return m_is_loaded; }

protected:
    void RenderAudioBlock(
        float* output_buffer,
        size_t num_frames,
        double sample_rate,
        const std::vector<MidiMessage>& midi_events
    ) override {
        if (!m_synth) {
            std::fill_n(output_buffer, num_frames * 2, 0.0f);
            return;
        }

        size_t current_frame = 0;
        size_t event_idx = 0;

        // Sample-accurate sub-block rendering loop
        while (current_frame < num_frames) {
            // Process all events scheduled at or before current_frame
            while (event_idx < midi_events.size() && midi_events[event_idx].frame_offset <= current_frame) {
                const auto& msg = midi_events[event_idx++];
                switch (msg.type) {
                    case MidiMessageType::NoteOn:
                        tsf_note_on(m_synth, 0, msg.data1, static_cast<float>(msg.data2) / 127.0f);
                        break;
                    case MidiMessageType::NoteOff:
                        tsf_note_off(m_synth, 0, msg.data1);
                        break;
                    case MidiMessageType::ControlChange:
                        if (msg.data1 == 123) tsf_reset(m_synth);
                        break;
                    default:
                        break;
                }
            }

            // Find next event target frame boundary
            size_t next_frame = num_frames;
            if (event_idx < midi_events.size()) {
                next_frame = std::min(num_frames, static_cast<size_t>(midi_events[event_idx].frame_offset));
            }

            // Render audio chunk up to the next frame offset
            size_t frames_to_render = next_frame - current_frame;
            if (frames_to_render > 0) {
                tsf_render_float(m_synth, output_buffer + (current_frame * 2), static_cast<int>(frames_to_render), 0);
                current_frame = next_frame;
            }
        }
    }

private:
    tsf* m_synth{nullptr};
    bool m_is_loaded{false};
};

} // namespace gsr::audio