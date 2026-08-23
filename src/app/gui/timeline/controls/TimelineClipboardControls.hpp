#pragma once

#include "ITimelineControls.hpp"

namespace gsr::gui {

class TimelineClipboardControls : public ITimelineControls {
public:
    void HandleKeyboardShortcuts(TimelineEditorContext& ctx) override;
    void DrawContextMenu(TimelineEditorContext& ctx) override;

private:
    static bool ClipsOverlap(uint64_t start1, uint64_t dur1, uint64_t start2, uint64_t dur2);
};

} // namespace gsr::gui