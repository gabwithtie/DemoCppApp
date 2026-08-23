// src/app/audio/AudioEngine.hpp
#pragma once
#include "TrackProcessor.hpp"
#include <vector>
#include <memory>
#include <cstdint>

#include "miniaudio.h" // Include miniaudio header

namespace gsr {
class App;

namespace audio {

class AudioEngine {
public:
    explicit AudioEngine(App& app);
    ~AudioEngine();

    bool Init(uint32_t sample_rate = 44100, uint32_t buffer_size = 512);
    void Shutdown();

    void AudioCallback(float* output_buffer, uint32_t frame_count);

    std::vector<std::unique_ptr<TrackProcessor>>& GetTrackProcessors() {
        return m_track_processors;
    }

    void AllNotesOff();

private:
    App& m_app;
    uint32_t m_sample_rate{44100};
    std::vector<std::unique_ptr<TrackProcessor>> m_track_processors;

    // Hardware device state
    ma_device m_device{};
    bool m_is_initialized{false};

    void SyncTrackProcessors();
    void ProcessPlaybackMidi(uint64_t start_tick, uint64_t end_tick, uint32_t frame_count);
};

} // namespace audio
} // namespace gsr