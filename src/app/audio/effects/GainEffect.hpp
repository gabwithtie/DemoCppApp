#pragma once

#include "Effect.hpp"
#include <algorithm>

namespace gsr::audio::effects {

class GainEffect final : public Effect {
public:
    [[nodiscard]] const char* Name() const override { return "Gain"; }

    void Process(std::vector<float>& interleaved_buffer,
                 size_t num_frames,
                 double /*sample_rate*/,
                 const app::GraphNodeData& node) override {
        const float gain = std::clamp(node.parameter_1, 0.0f, 2.0f);
        const size_t sample_count = num_frames * 2;
        for (size_t sample = 0; sample < sample_count; ++sample) {
            interleaved_buffer[sample] *= gain;
        }
    }
};

} // namespace gsr::audio::effects
