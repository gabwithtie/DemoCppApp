#pragma once

#include "IInstrument.hpp"
#include <algorithm>
#include <memory>
#include <vector>

namespace gsr::audio {

class TrackProcessor {
public:
    TrackProcessor() = default;

    void SetInstrument(std::unique_ptr<IInstrument> instrument) {
        m_instrument = std::move(instrument);
    }

    [[nodiscard]] bool HasInstrument() const { return m_instrument != nullptr; }
    [[nodiscard]] IInstrument* GetInstrument() const { return m_instrument.get(); }

    void ProcessAudioBlock(float* mix_output_buffer, size_t num_frames, double sample_rate, float track_volume) {
        // Pre-allocated scratch buffer resize (guaranteed lock-free if capacity is sufficient)
        if (m_scratch_buffer.size() < num_frames * 2) {
            m_scratch_buffer.resize(num_frames * 2, 0.0f);
        }

        if (m_instrument) {
            m_instrument->ProcessAndRender(m_scratch_buffer.data(), num_frames, sample_rate);
        } else {
            std::fill_n(m_scratch_buffer.data(), num_frames * 2, 0.0f);
        }

        // Mix track scratch buffer into master output with volume control
        for (size_t i = 0; i < num_frames * 2; ++i) {
            mix_output_buffer[i] += m_scratch_buffer[i] * track_volume;
        }
    }

private:
    std::unique_ptr<IInstrument> m_instrument;
    std::vector<float> m_scratch_buffer; // Reusable buffer to avoid heap allocations in audio loop
};

} // namespace gsr::audio