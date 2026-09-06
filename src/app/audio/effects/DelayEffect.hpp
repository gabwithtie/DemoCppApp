#pragma once

#include "Effect.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace gsr::audio::effects {

class DelayEffect final : public Effect {
public:
    [[nodiscard]] const char* Name() const override { return "Delay"; }

    void Process(std::vector<float>& interleaved_buffer,
                 size_t num_frames,
                 double sample_rate,
                 const app::GraphNodeData& node) override {
        const float wet = std::clamp(node.parameter_1, 0.0f, 1.0f);
        const size_t delay_frames = std::max<size_t>(1, static_cast<size_t>(std::floor(sample_rate * 0.25)));

        if (m_delay_buffer.size() != delay_frames * 2) {
            m_delay_buffer.assign(delay_frames * 2, 0.0f);
            m_delay_index = 0;
        }

        for (size_t frame = 0; frame < num_frames; ++frame) {
            const size_t in_index = frame * 2;
            const size_t delay_index = m_delay_index * 2;

            const float delayed_left = m_delay_buffer[delay_index];
            const float delayed_right = m_delay_buffer[delay_index + 1];
            const float input_left = interleaved_buffer[in_index];
            const float input_right = interleaved_buffer[in_index + 1];

            m_delay_buffer[delay_index] = input_left;
            m_delay_buffer[delay_index + 1] = input_right;

            interleaved_buffer[in_index] = input_left * (1.0f - wet) + delayed_left * wet;
            interleaved_buffer[in_index + 1] = input_right * (1.0f - wet) + delayed_right * wet;

            m_delay_index = (m_delay_index + 1) % delay_frames;
        }
    }

private:
    std::vector<float> m_delay_buffer;
    size_t m_delay_index{0};
};

} // namespace gsr::audio::effects
