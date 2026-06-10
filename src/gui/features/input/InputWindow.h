#pragma once

#include "gui/main/GuiWindow.h"
#include "InputAction.h"
#include "InputBinding.h"
#include <string>

namespace app {

    // ---------------------------------------------------------------------------
    // InputWindow
    //
    // A GuiWindow that lets the user view and remap every InputAction binding.
    // Saves to disk automatically when closed or when the user clicks Apply.
    //
    // Typical setup in your app root:
    //   m_input_window = std::make_unique<InputWindow>("input_bindings.json");
    // ---------------------------------------------------------------------------
    class InputWindow : public GuiWindow {
    public:
        InputWindow(std::string config_path);

        std::string GetWindowId() override { return "Input Bindings##app"; }

        // Override so we can auto-save when the window is closed from outside.
        void SetOpen(bool newstate) override;

    protected:
        void DrawSelf() override;

    private:
        std::string  m_config_path;

        // Which action is currently waiting for a new binding (-1 = none).
        int          m_capturing_action_idx = -1;

        // Human-readable names for each action (index matches InputAction enum).
        static const char* ActionDisplayName(InputAction action);

        void DrawActionRow(InputAction action, int idx);
        void ApplyAndSave();
    };

} // namespace app
