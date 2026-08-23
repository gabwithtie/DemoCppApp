#pragma once

#include "App.hpp"
#include "model/Track.hpp"
#include <imgui.h>

namespace gsr::gui {

struct TimelineEditorContext {
    gsr::App& app;
    Model::Track& track;
    size_t track_index;
    uint32_t ticks_per_bar;
};

class ITimelineControls {
public:
    virtual ~ITimelineControls() = default;
    virtual void HandleKeyboardShortcuts(TimelineEditorContext& ctx) = 0;
    virtual void DrawContextMenu(TimelineEditorContext& ctx) = 0;
};

} // namespace gsr::gui