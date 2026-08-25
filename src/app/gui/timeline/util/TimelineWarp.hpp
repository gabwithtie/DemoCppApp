#pragma once

#include "model/Project.hpp"
#include "App.hpp"
#include <algorithm>

namespace gsr::gui {

inline double SourceToTargetTick(const Model::Project& project, double source_tick) {
    if (project.time_keys.empty()) {
        return source_tick;
    }

    const auto& keys = project.time_keys;
    if (source_tick <= keys.front().source_tick) {
        return source_tick + static_cast<double>(keys.front().target_tick) - keys.front().source_tick;
    }

    for (size_t i = 1; i < keys.size(); ++i) {
        const auto& previous = keys[i - 1];
        const auto& current = keys[i];
        if (source_tick <= current.source_tick && current.source_tick > previous.source_tick) {
            const double amount = (source_tick - previous.source_tick) /
                                  static_cast<double>(current.source_tick - previous.source_tick);
            return previous.target_tick + amount *
                   static_cast<double>(current.target_tick - previous.target_tick);
        }
    }

    return keys.back().target_tick + source_tick - keys.back().source_tick;
}

inline double TargetToSourceTick(const Model::Project& project, double target_tick) {
    if (project.time_keys.empty()) {
        return target_tick;
    }

    const auto& keys = project.time_keys;
    if (target_tick <= keys.front().target_tick) {
        return target_tick + static_cast<double>(keys.front().source_tick) - keys.front().target_tick;
    }

    for (size_t i = 1; i < keys.size(); ++i) {
        const auto& previous = keys[i - 1];
        const auto& current = keys[i];
        if (target_tick <= current.target_tick && current.target_tick > previous.target_tick) {
            const double amount = (target_tick - previous.target_tick) /
                                  static_cast<double>(current.target_tick - previous.target_tick);
            return previous.source_tick + amount *
                   static_cast<double>(current.source_tick - previous.source_tick);
        }
    }

    return keys.back().source_tick + target_tick - keys.back().target_tick;
}

inline void RecalculateTempoFromTimeKeys(Model::Project& project, Transport& transport) {
    // Fallback: Default BPM when no keys exist
    if (project.time_keys.empty()) {
        project.tempo_map.clear();
        project.tempo_map.push_back({ 0, project.default_bpm });
        transport.bpm = project.default_bpm;
        return;
    }

    // Keep the source timeline ordered so interpolation remains deterministic.
    std::sort(project.time_keys.begin(), project.time_keys.end(),
        [](const Model::TimeKey& a, const Model::TimeKey& b) {
            return a.source_tick < b.source_tick;
        });

    for (size_t i = 1; i < project.time_keys.size(); ++i) {
        if (project.time_keys[i].target_tick <= project.time_keys[i - 1].target_tick) {
            project.time_keys[i].target_tick = project.time_keys[i - 1].target_tick + 1;
        }
    }

    project.tempo_map.clear();

    // Standard base tempo up to the first handle if offset
    if (project.time_keys.front().target_tick > 0) {
        project.tempo_map.push_back({ 0, project.default_bpm });
    }

    // A stretched target segment has a proportionally slower tempo.
    for (size_t i = 0; i < project.time_keys.size(); ++i) {
        const auto& current = project.time_keys[i];
        double calculated_bpm = project.default_bpm;

        if (i + 1 < project.time_keys.size()) {
            const auto& next = project.time_keys[i + 1];
            
            double delta_source = static_cast<double>(next.source_tick) - static_cast<double>(current.source_tick);
            double delta_target = static_cast<double>(next.target_tick) - static_cast<double>(current.target_tick);

            if (delta_target > 0.0 && delta_source > 0.0) {
                calculated_bpm = project.default_bpm * (delta_source / delta_target);
            }
        }

        project.tempo_map.push_back({ current.target_tick, calculated_bpm });
    }

    if (!project.tempo_map.empty()) {
        transport.bpm = project.tempo_map.front().bpm;
    }
}

} // namespace gsr::gui