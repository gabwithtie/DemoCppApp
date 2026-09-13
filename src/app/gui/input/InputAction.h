#pragma once

namespace app {

    // Every logical input action in the application.
    // Add new actions here; bindings are configured in InputMap.
    enum class InputAction {
        // --- Viewport Navigation ---
        Viewport_Zoom,          // scroll wheel axis
        Viewport_Pan,           // middle-mouse drag delta

        // --- Viewport Tools ---
        Tool_PrimaryUse,        // left mouse held  (e.g. brush stroke)
        Tool_SecondaryUse,      // right mouse held (e.g. erase / eyedrop)
        Tool_CyclePrev,         // key: cycle tool backwards
        Tool_CycleNext,         // key: cycle tool forward

        // --- Canvas ---
        Canvas_Save,            // key: manual save
        Canvas_Undo,            // key
        Canvas_Redo,            // key

        // --- General ---
        UI_ToggleInputWindow,   // key: open/close the binding editor

        _Count  // sentinel — keep last
    };

} // namespace app
