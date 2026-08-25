#define MINIAUDIO_IMPLEMENTATION
#include "tsf.h"

#include "AudioEngine.hpp"
#include "App.hpp"
#include "SoundFontInstrument.hpp"
#include "gui/timeline/util/TimelineWarp.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace gsr::audio {

static void miniaudio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    auto* engine = static_cast<AudioEngine*>(pDevice->pUserData);
    if (engine) {
        engine->AudioCallback(static_cast<float*>(pOutput), frameCount);
    }
}

AudioEngine::AudioEngine(App& app) : m_app(app) {}
AudioEngine::~AudioEngine() { Shutdown(); }

bool AudioEngine::Init(uint32_t sample_rate, uint32_t buffer_size) {
    m_sample_rate = sample_rate;

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = sample_rate;
    config.periodSizeInFrames = buffer_size;
    config.dataCallback = miniaudio_data_callback;
    config.pUserData = this;

    if (ma_device_init(NULL, &config, &m_device) != MA_SUCCESS) {
        return false;
    }

    if (ma_device_start(&m_device) != MA_SUCCESS) {
        ma_device_uninit(&m_device);
        return false;
    }

    m_is_initialized = true;
    SyncTrackProcessors();
    return true;
}

void AudioEngine::Shutdown() {
    if (m_is_initialized) {
        ma_device_uninit(&m_device);
        m_is_initialized = false;
    }
}

void AudioEngine::SyncTrackProcessors() {
    if (m_track_processors.size() != m_app.project.tracks.size()) {
        m_track_processors.clear();
    }

    while (m_track_processors.size() < m_app.project.tracks.size()) {
        m_track_processors.push_back(std::make_unique<TrackProcessor>());
    }

    for (size_t i = 0; i < m_app.project.tracks.size(); ++i) {
        auto& track = m_app.project.tracks[i];
        auto& processor = m_track_processors[i];

        // Assign instrument if missing or flag requested a reload
        if (!processor->HasInstrument() || track.needs_reload) {
            if (!track.soundfont_path.empty()) {
                auto inst = std::make_unique<SoundFontInstrument>();
                inst->LoadFile(track.soundfont_path);
                processor->SetInstrument(std::move(inst));
            }
            track.needs_reload = false;
        }
    }
}

void AudioEngine::ProcessPlaybackMidi(uint64_t start_tick, uint64_t end_tick, uint32_t frame_count) {
    uint64_t tick_delta = end_tick - start_tick;
    if (tick_delta == 0) return;

    const double frames_per_target_tick = static_cast<double>(frame_count) / static_cast<double>(tick_delta);

    for (size_t i = 0; i < m_app.project.tracks.size(); ++i) {
        auto& track = m_app.project.tracks[i];
        if (track.muted || i >= m_track_processors.size()) continue;

        auto* instrument = m_track_processors[i]->GetInstrument();
        if (!instrument) continue;

        for (const auto& clip : track.clips) {
            const double clip_start = gui::SourceToTargetTick(m_app.project, clip.start_tick);
            const double clip_end = gui::SourceToTargetTick(m_app.project, clip.start_tick + clip.duration);

            if (clip_end < start_tick || clip_start > end_tick) continue;

            for (const auto& note : clip.notes) {
                const uint64_t source_note_start = clip.start_tick + note.start_tick;
                const double abs_note_start = gui::SourceToTargetTick(m_app.project, source_note_start);

                // Ignore notes starting outside clip bounds
                if (abs_note_start >= clip_end) continue;

                // Truncate NoteOff at clip_end if note.duration extends past the clip boundary
                const uint64_t source_note_end = source_note_start + note.duration;
                const double raw_note_end = gui::SourceToTargetTick(m_app.project, source_note_end);
                const double effective_note_end = std::min(raw_note_end, clip_end);

                // Send NoteOn
                if (abs_note_start >= start_tick && abs_note_start < end_tick) {
                    uint32_t offset = static_cast<uint32_t>((abs_note_start - start_tick) * frames_per_target_tick);
                    instrument->SendNoteOn(track.midi_channel, note.pitch, note.velocity, offset);
                }

                // Send NoteOff at truncated boundary
                if (effective_note_end >= start_tick && effective_note_end < end_tick) {
                    uint32_t offset = static_cast<uint32_t>((effective_note_end - start_tick) * frames_per_target_tick);
                    instrument->SendNoteOff(track.midi_channel, note.pitch, 0, offset);
                }
            }
        }
    }
}

void AudioEngine::AudioCallback(float* output_buffer, uint32_t frame_count) {
    std::fill_n(output_buffer, frame_count * 2, 0.0f);

    SyncTrackProcessors();

    double seconds_per_buffer = static_cast<double>(frame_count) / m_sample_rate;
    double target_ticks_per_second = (m_app.project.default_bpm / 60.0) * m_app.project.ppq;

    if (m_app.transport.state == PlaybackState::Playing) {
        uint64_t start_tick = m_app.transport.current_tick.load(std::memory_order_relaxed);
        uint64_t buffer_ticks = static_cast<uint64_t>(std::ceil(target_ticks_per_second * seconds_per_buffer));
        if (buffer_ticks == 0) buffer_ticks = 1;
        uint64_t end_tick = start_tick + buffer_ticks;

        ProcessPlaybackMidi(start_tick, end_tick, frame_count);
        m_app.transport.current_tick.store(end_tick, std::memory_order_relaxed);
    }

    // Render & Mix Track Audio
    for (size_t i = 0; i < m_app.project.tracks.size(); ++i) {
        auto& track = m_app.project.tracks[i];
        if (track.muted || i >= m_track_processors.size()) continue;

        //TODO: Handle instrument change

        m_track_processors[i]->ProcessAudioBlock(output_buffer, frame_count, m_sample_rate, track.volume);
    }
}

void AudioEngine::AllNotesOff() {
    for (size_t i = 0; i < m_track_processors.size(); ++i) {
        if (i >= m_app.project.tracks.size()) continue;

        auto* instrument = m_track_processors[i]->GetInstrument();
        if (!instrument) continue;

        uint8_t ch = m_app.project.tracks[i].midi_channel;

        // Flush all active pitch states on the track's MIDI channel
        for (uint8_t pitch = 0; pitch < 128; ++pitch) {
            instrument->SendNoteOff(ch, pitch, 0, 0);
        }
    }
}

} // namespace gsr::audio