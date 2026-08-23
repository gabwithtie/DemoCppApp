#pragma once

#include "IInstrument.hpp"
#include "TypeRegistry.hpp"
#include "gui/util/WarningModal.hpp"
#include <tsf.h>

namespace gsr::audio {

class SoundFontInstrument : public IInstrument {
public:
    SoundFontInstrument() { BindProperties(); }
    explicit SoundFontInstrument(gbe::SerializedData& data) : IInstrument(data) { BindProperties(); }
    ~SoundFontInstrument() {
        if (m_synth) tsf_close(m_synth);
    }

    void LoadFile(const std::string& path) {
        if (path.empty()) return;
        
        if (m_synth) tsf_close(m_synth);
        m_synth = tsf_load_filename(path.c_str());
        
        if (!m_synth) {
            WarningModal::Trigger("Failed to load SoundFont file:\n" + path);
            return;
        }
        tsf_set_output(m_synth, TSF_STEREO_INTERLEAVED, 44100, 0.0f);
    }

protected:
    void RenderAudioBlock(float* out, size_t frames, double sr, const std::vector<MidiMessage>& events) override {
        if (!m_synth) {
            std::fill_n(out, frames * 2, 0.0f);
            return;
        }

        tsf_set_output(m_synth, TSF_STEREO_INTERLEAVED, static_cast<int>(sr), 0.0f);
        size_t current_frame = 0;

        for (const auto& msg : events) {
            uint32_t event_frame = std::min(msg.frame_offset, static_cast<uint32_t>(frames));

            // Render audio up to the frame offset of this event
            if (event_frame > current_frame) {
                size_t frames_to_render = event_frame - current_frame;
                tsf_render_float(m_synth, out + (current_frame * 2), static_cast<int>(frames_to_render), 0);
                current_frame = event_frame;
            }

            // Process MIDI message
            if (msg.type == MidiMessageType::NoteOn) {
                tsf_note_on(m_synth, 0, msg.data1, static_cast<float>(msg.data2) / 127.0f);
            } else if (msg.type == MidiMessageType::NoteOff) {
                tsf_note_off(m_synth, 0, msg.data1);
            }
        }

        // Render remaining frames in the block
        if (current_frame < frames) {
            size_t remaining_frames = frames - current_frame;
            tsf_render_float(m_synth, out + (current_frame * 2), static_cast<int>(remaining_frames), 0);
        }
    }

private:
    std::string m_path;
    tsf* m_synth{nullptr};

    GBE_SERIALIZE_FIELD_W_NAME_CB(m_path, "SoundFont Path", [this](std::string& new_path) {
        LoadFile(new_path);
    });

    void BindProperties() {
        if (!m_path.empty()) LoadFile(m_path);
    }
};

GBE_REGISTER_SERIALIZED_TYPE(SoundFontInstrument, IInstrument);

} // namespace gsr::audio