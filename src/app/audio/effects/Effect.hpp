#pragma once

#include "../../../gui/features/nodeeditor/GenericNodeEditor.hpp"
#include <cstddef>
#include <vector>

namespace gsr::audio::effects {

class Effect {
public:
    virtual ~Effect() = default;

    [[nodiscard]] virtual const char* Name() const = 0;
    virtual void Process(std::vector<float>& interleaved_buffer,
                         size_t num_frames,
                         double sample_rate,
                         const app::GraphNodeData& node) = 0;
};

} // namespace gsr::audio::effects
