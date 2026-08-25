#pragma once

#include "model/Project.hpp"
#include "App.hpp"
#include <algorithm>

namespace gsr::gui {

inline void RecalculateTempoFromTimeKeys(Model::Project& project, Transport& transport) {
    // Fallback: Default BPM when no keys exist
    if (project.time_keys.empty()) {
        project.tempo_map.clear();
        project.tempo_map.push_back({ 0, project.default_bpm });
        transport.bpm = project.default_bpm;
        return;
    }

    // Keep keys chronologically sorted by target timeline position
    std::sort(project.time_keys.begin(), project.time_keys.end(),
        [](const Model::TimeKey& a, const Model::TimeKey& b) {
            return a.target_tick < b.target_tick;
        });

    project.tempo_map.clear();

    // Standard base tempo up to the first handle if offset
    if (project.time_keys.front().target_tick > 0) {
        project.tempo_map.push_back({ 0, project.default_bpm });
    }

    // Calculate dynamic segment BPM ratios
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