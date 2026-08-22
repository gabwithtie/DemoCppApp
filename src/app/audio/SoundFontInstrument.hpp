// src/app/audio/SoundFontInstrument.hpp
#pragma once

#define TSF_IMPLEMENTATION

#include "IInstrument.hpp"
#include <tsf.h>
#include <string>
#include <filesystem>
#include <iostream>

namespace gsr::audio {

class SoundFontInstrument : public IInstrument {
public:
    explicit SoundFontInstrument(const std::string& path, double sample_rate = 44100.0) {
        // 1. Check if the string path is empty
        if (path.empty()) {
            std::cerr << "[SoundFont Error] Cannot load instrument: Path is empty.\n";
            return;
        }

        // 2. Verify file exists on the filesystem
        if (!std::filesystem::exists(path)) {
            std::cerr << "[SoundFont Error] File not found at path: " << path << "\n";
            return;
        }

        // 3. Attempt to load the SoundFont binary
        m_synth = tsf_load_filename(path.c_str());
        if (!m_synth) {
            std::cerr << "[SoundFont Error] Failed to parse SoundFont file (file may be corrupted or invalid .sf2): " 
                      << path << "\n";
            return;
        }

        // 4. Configure synthesizer output mode
        tsf_set_output(m_synth, TSF_STEREO_INTERLEAVED, static_cast<int>(sample_rate), 0.0f);
        m_is_loaded = true;
        std::cout << "[SoundFont Success] Loaded SF2: " << std::filesystem::path(path).filename().string() << "\n";
    }

    ~SoundFontInstrument() override {
        if (m_synth) {
            tsf_close(m_synth);
            m_synth = nullptr;
        }
    }

    [[nodiscard]] bool IsLoaded() const { return m_is_loaded; }

    void ProcessMidiEvent(const midi::MidiMessage& msg) override {
        // Silent return on uninitialized state to avoid locking/logging on the audio thread
        if (!m_synth) return;

        switch (msg.type) {
            case midi::MidiMessageType::NoteOn:
                tsf_note_on(m_synth, 0, msg.data1, static_cast<float>(msg.data2) / 127.0f);
                break;
            case midi::MidiMessageType::NoteOff:
                tsf_note_off(m_synth, 0, msg.data1);
                break;
            case midi::MidiMessageType::ControlChange:
                if (msg.data1 == 123) tsf_reset(m_synth);
                break;
            default:
                break;
        }
    }

    void RenderAudioBlock(float* output_buffer, size_t num_frames, double sample_rate) override {
        if (!m_synth) {
            std::fill_n(output_buffer, num_frames * 2, 0.0f);
            return;
        }
        tsf_render_float(m_synth, output_buffer, static_cast<int>(num_frames), 0);
    }

private:
    tsf* m_synth{nullptr};
    bool m_is_loaded{false};
};

} // namespace gsr::audio