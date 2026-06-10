#pragma once

#include <imgui.h>
#include <string>

namespace app {

    // What physical input drives this action?
    enum class BindingType {
        None,
        Key,            // ImGuiKey (pressed or held)
        MouseButton,    // ImGuiMouseButton (clicked or held)
        MouseScroll,    // mouse wheel float axis
        MouseDrag,      // mouse button drag delta (ImVec2)
    };

    struct InputBinding {
        BindingType type = BindingType::None;

        // Used by Key and MouseButton
        ImGuiKey         key         = ImGuiKey_None;
        ImGuiMouseButton mouseButton = ImGuiMouseButton_Left;

        // Required modifier keys. All that are true must be held for the
        // binding to match. False means the modifier must NOT be held.
        // (Use the factory WithModifiers() helper or set directly.)
        bool requireCtrl  = false;
        bool requireShift = false;
        bool requireAlt   = false;

        // Display / serialization label — auto-built by BuildLabel(), or set manually.
        std::string label;

        // Rebuilds the label string from the current modifier + key/button state.
        // Call after constructing or mutating a binding so the UI stays in sync.
        void BuildLabel() {
            label.clear();
            if (requireCtrl)  label += "Ctrl+";
            if (requireShift) label += "Shift+";
            if (requireAlt)   label += "Alt+";

            switch (type) {
                case BindingType::Key:
                    label += ImGui::GetKeyName(key);
                    break;
                case BindingType::MouseButton:
                    label += MouseButtonName(mouseButton);
                    break;
                case BindingType::MouseScroll:
                    label += "Scroll";
                    break;
                case BindingType::MouseDrag:
                    label += MouseButtonName(mouseButton) + " Drag";
                    break;
                default:
                    label = "(unbound)";
                    break;
            }
        }

        // Fluent modifier setter — use after a From*() call:
        //   InputBinding::FromKey(ImGuiKey_S).WithModifiers(true, false, false)
        InputBinding WithModifiers(bool ctrl, bool shift, bool alt) {
            requireCtrl  = ctrl;
            requireShift = shift;
            requireAlt   = alt;
            BuildLabel();
            return *this;
        }

        // --- Factory helpers ---
        static InputBinding FromKey(ImGuiKey k) {
            InputBinding b;
            b.type = BindingType::Key;
            b.key  = k;
            b.BuildLabel();
            return b;
        }

        static InputBinding FromMouseButton(ImGuiMouseButton btn) {
            InputBinding b;
            b.type        = BindingType::MouseButton;
            b.mouseButton = btn;
            b.BuildLabel();
            return b;
        }

        static InputBinding FromScroll() {
            InputBinding b;
            b.type = BindingType::MouseScroll;
            b.BuildLabel();
            return b;
        }

        static InputBinding FromDrag(ImGuiMouseButton btn) {
            InputBinding b;
            b.type        = BindingType::MouseDrag;
            b.mouseButton = btn;
            b.BuildLabel();
            return b;
        }

        static InputBinding None() { return InputBinding{}; }

    private:
        static std::string MouseButtonName(ImGuiMouseButton btn) {
            switch (btn) {
                case ImGuiMouseButton_Left:   return "LMB";
                case ImGuiMouseButton_Right:  return "RMB";
                case ImGuiMouseButton_Middle: return "MMB";
                default:                      return "MB" + std::to_string(btn);
            }
        }
    };

} // namespace app
