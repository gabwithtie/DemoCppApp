#pragma once

#include "controls/ITimelineControls.hpp"
#include "controls/TimelineSelectionControls.hpp"
#include "controls/TimelineClipboardControls.hpp"
#include "controls/TimelineClipEditControls.hpp"
#include <vector>

namespace gsr::gui {

class TimelineInteraction {
public:
    TimelineInteraction();
    ~TimelineInteraction() = default;

    void HandleKeyboardShortcuts(TimelineEditorContext& ctx);
    void DrawContextMenu(TimelineEditorContext& ctx);

private:
    TimelineSelectionControls m_selection_controls;
    TimelineClipboardControls m_clipboard_controls;
    TimelineClipEditControls  m_clip_edit_controls;

    std::vector<ITimelineControls*> m_controls;
};

} // namespace gsr::gui