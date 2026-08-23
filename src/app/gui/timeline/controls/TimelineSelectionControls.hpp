#pragma once

#include "ITimelineControls.hpp"

namespace gsr::gui {

class TimelineSelectionControls : public ITimelineControls {
public:
    void HandleKeyboardShortcuts(TimelineEditorContext& ctx) override;
    void DrawContextMenu(TimelineEditorContext& ctx) override {}
};

} // namespace gsr::gui