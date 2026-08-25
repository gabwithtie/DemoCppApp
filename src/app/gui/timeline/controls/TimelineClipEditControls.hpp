#pragma once

#include "ITimelineControls.hpp"

namespace gsr::gui {

class TimelineClipEditControls : public ITimelineControls {
public:
    void HandleKeyboardShortcuts(TimelineEditorContext& ctx) override;
    void DrawContextMenu(TimelineEditorContext& ctx) override;

private:
    static bool ImportMidiAtSelection(TimelineEditorContext& ctx);
};

} // namespace gsr::gui