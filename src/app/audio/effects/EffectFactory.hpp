#pragma once

#include "DelayEffect.hpp"
#include "GainEffect.hpp"
#include "ReverbEffect.hpp"
#include <memory>
#include <string>

namespace gsr::audio::effects {

inline std::unique_ptr<Effect> CreateEffectByName(const std::string& effect_name) {
    if (effect_name == "Gain") {
        return std::make_unique<GainEffect>();
    }
    if (effect_name == "Reverb") {
        return std::make_unique<ReverbEffect>();
    }
    if (effect_name == "Delay") {
        return std::make_unique<DelayEffect>();
    }
    return nullptr;
}

} // namespace gsr::audio::effects
