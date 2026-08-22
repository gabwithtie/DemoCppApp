#pragma once

#include "App.hpp"
#include "model/Track.hpp"
#include <imgui.h>

namespace gsr::gui {

class ClipManager {
public:
    ClipManager() = default;
    ~ClipManager() = default;

    void DrawRuler(gsr::App& app, float ruler_height);
    void DrawTrackTimeline(gsr::App& app, Model::Track& track, size_t track_index, float row_height);

private:
    static bool ClipsOverlap(uint64_t start1, uint64_t dur1, uint64_t start2, uint64_t dur2);
};

} // namespace gsr::gui