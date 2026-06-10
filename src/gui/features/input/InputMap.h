#pragma once

#include "InputAction.h"
#include "InputBinding.h"

#include <imgui.h>
#include <array>
#include <string>

namespace app {

    // ---------------------------------------------------------------------------
    // InputMap  —  singleton input abstraction layer
    //
    // Usage:
    //   InputMap::Get().IsHeld(InputAction::Tool_PrimaryUse)
    //   InputMap::Get().GetScrollAxis(InputAction::Viewport_Zoom)
    //   InputMap::Get().GetDragDelta(InputAction::Viewport_Pan)
    //
    // Call LoadFromFile() at startup and SaveToFile() when the user edits bindings.
    // BeginCapture() / PollCapture() drive the InputWindow remap flow.
    // ---------------------------------------------------------------------------
    class InputMap {
    public:
        static InputMap& Get();

        // --- Lifecycle ---
        void LoadFromFile(const std::string& path);
        void SaveToFile(const std::string& path) const;
        void ResetToDefaults();

        // --- Binding access ---
        const InputBinding& GetBinding(InputAction action) const;
        void                SetBinding(InputAction action, InputBinding binding);

        // --- Query API (call each frame inside ImGui context) ---

        // True while the bound key / mouse button is held down.
        bool IsHeld(InputAction action) const;

        // True on the frame the bound key / mouse button was first pressed.
        bool IsPressed(InputAction action) const;

        // Scroll-wheel value this frame (0 if unbound or not a scroll action).
        float GetScrollAxis(InputAction action) const;

        // Drag delta this frame (zero if unbound or not a drag action).
        ImVec2 GetDragDelta(InputAction action) const;

        // --- Capture / remap flow (used by InputWindow) ---
        // Start listening for the next physical input.
        void BeginCapture();
        // Returns true and writes result once an input is detected, false while waiting.
        bool PollCapture(InputBinding& out_binding);
        bool IsCapturing() const { return m_capturing; }

    private:
        InputMap();

        std::array<InputBinding, static_cast<size_t>(InputAction::_Count)> m_bindings;

        bool         m_capturing        = false;
        InputBinding m_captured_binding = InputBinding::None();
        bool         m_capture_ready    = false;

        // Scans ImGuiIO for any physical input and fills m_captured_binding.
        void TickCapture();
    };

} // namespace app
