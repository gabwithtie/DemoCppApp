#include "InputMap.h"

#include <imgui.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Minimal JSON helpers (no external dependency).
// Swap for nlohmann/json or similar if already in your project.
// ---------------------------------------------------------------------------
namespace {

    std::string JsonString(const std::string& v) {
        return "\"" + v + "\"";
    }

    // Very small hand-rolled writer — only needs to emit flat key/value pairs.
    struct JsonWriter {
        std::ostringstream ss;
        bool first = true;

        void Begin() { ss << "{\n"; }
        void End()   { ss << "\n}\n"; }

        void Write(const std::string& key, const std::string& val) {
            if (!first) ss << ",\n";
            ss << "  " << JsonString(key) << ": " << JsonString(val);
            first = false;
        }

        std::string Str() const { return ss.str(); }
    };

    // Tiny reader: extracts "key": "value" pairs from a flat JSON object.
    // Good enough for our config; replace with a real parser for nested data.
    std::string ExtractJsonValue(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\"";
        auto pos = json.find(search);
        if (pos == std::string::npos) return "";
        pos = json.find(':', pos);
        if (pos == std::string::npos) return "";
        pos = json.find('"', pos);
        if (pos == std::string::npos) return "";
        ++pos;
        auto end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Serialization keys  (one per InputAction entry)
// ---------------------------------------------------------------------------
namespace {

    // Returns a stable string key for each action used in JSON.
    const char* ActionKey(app::InputAction a) {
        using A = app::InputAction;
        switch (a) {
            case A::Viewport_Zoom:         return "Viewport_Zoom";
            case A::Viewport_Pan:          return "Viewport_Pan";
            case A::Tool_PrimaryUse:       return "Tool_PrimaryUse";
            case A::Tool_SecondaryUse:     return "Tool_SecondaryUse";
            case A::Tool_CyclePrev:        return "Tool_CyclePrev";
            case A::Tool_CycleNext:        return "Tool_CycleNext";
            case A::Canvas_Save:           return "Canvas_Save";
            case A::Canvas_Undo:           return "Canvas_Undo";
            case A::Canvas_Redo:           return "Canvas_Redo";
            case A::UI_ToggleInputWindow:  return "UI_ToggleInputWindow";
            default:                       return "Unknown";
        }
    }

    // Serialises a binding to a compact string.
    // Format:  <type_token>[|C][|S][|A]
    // Examples:
    //   "Key:S|C"     ->  Ctrl+S
    //   "MB:0|S|A"    ->  Shift+Alt+LMB
    //   "Scroll"      ->  scroll (modifiers not applicable)
    //   "Drag:2|C"    ->  Ctrl+MMB drag
    //   "None"        ->  unbound
    std::string SerializeBinding(const app::InputBinding& b) {
        using BT = app::BindingType;
        std::string base;
        switch (b.type) {
            case BT::Key:         base = std::string("Key:")  + ImGui::GetKeyName(b.key); break;
            case BT::MouseButton: base = std::string("MB:")   + std::to_string(b.mouseButton); break;
            case BT::MouseScroll: return "Scroll";
            case BT::MouseDrag:   base = std::string("Drag:") + std::to_string(b.mouseButton); break;
            default:              return "None";
        }
        if (b.requireCtrl)  base += "|C";
        if (b.requireShift) base += "|S";
        if (b.requireAlt)   base += "|A";
        return base;
    }

    app::InputBinding DeserializeBinding(const std::string& s) {
        if (s == "None" || s.empty()) return app::InputBinding::None();
        if (s == "Scroll")            return app::InputBinding::FromScroll();

        // Parse modifier suffix flags before the type token.
        bool ctrl  = (s.find("|C") != std::string::npos);
        bool shift = (s.find("|S") != std::string::npos);
        bool alt   = (s.find("|A") != std::string::npos);

        // Strip flags to get the clean type token.
        std::string token = s.substr(0, s.find('|'));

        app::InputBinding result;

        if (token.rfind("Key:", 0) == 0) {
            std::string kname = token.substr(4);
            for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
                auto key = static_cast<ImGuiKey>(k);
                if (std::string(ImGui::GetKeyName(key)) == kname) {
                    result = app::InputBinding::FromKey(key);
                    break;
                }
            }
        } else if (token.rfind("MB:", 0) == 0) {
            int btn = std::stoi(token.substr(3));
            result = app::InputBinding::FromMouseButton(static_cast<ImGuiMouseButton>(btn));
        } else if (token.rfind("Drag:", 0) == 0) {
            int btn = std::stoi(token.substr(5));
            result = app::InputBinding::FromDrag(static_cast<ImGuiMouseButton>(btn));
        } else {
            return app::InputBinding::None();
        }

        return result.WithModifiers(ctrl, shift, alt);
    }

} // anonymous namespace

// ---------------------------------------------------------------------------
namespace app {

InputMap& InputMap::Get() {
    static InputMap instance;
    return instance;
}

InputMap::InputMap() {
    ResetToDefaults();
}

void InputMap::ResetToDefaults() {
    using A = InputAction;
    auto& b = m_bindings;

    b[static_cast<size_t>(A::Viewport_Zoom)]        = InputBinding::FromScroll();
    b[static_cast<size_t>(A::Viewport_Pan)]          = InputBinding::FromDrag(ImGuiMouseButton_Middle);
    b[static_cast<size_t>(A::Tool_PrimaryUse)]       = InputBinding::FromMouseButton(ImGuiMouseButton_Left);
    b[static_cast<size_t>(A::Tool_SecondaryUse)]     = InputBinding::FromMouseButton(ImGuiMouseButton_Right);
    b[static_cast<size_t>(A::Tool_CyclePrev)]        = InputBinding::FromKey(ImGuiKey_Q);
    b[static_cast<size_t>(A::Tool_CycleNext)]        = InputBinding::FromKey(ImGuiKey_E);
    b[static_cast<size_t>(A::Canvas_Save)]           = InputBinding::FromKey(ImGuiKey_S).WithModifiers(true,  false, false);
    b[static_cast<size_t>(A::Canvas_Undo)]           = InputBinding::FromKey(ImGuiKey_Z).WithModifiers(true,  false, false);
    b[static_cast<size_t>(A::Canvas_Redo)]           = InputBinding::FromKey(ImGuiKey_Y).WithModifiers(true,  false, false);
    b[static_cast<size_t>(A::UI_ToggleInputWindow)]  = InputBinding::FromKey(ImGuiKey_F2);
}

// --- Persistence -----------------------------------------------------------

void InputMap::LoadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return; // missing file is fine — defaults remain

    std::string json((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());

    for (size_t i = 0; i < static_cast<size_t>(InputAction::_Count); ++i) {
        const char* key = ActionKey(static_cast<InputAction>(i));
        std::string val = ExtractJsonValue(json, key);
        if (!val.empty()) {
            m_bindings[i] = DeserializeBinding(val);
            // Preserve the stored label if present (label is embedded in the value string itself via factory)
        }
    }
}

void InputMap::SaveToFile(const std::string& path) const {
    JsonWriter w;
    w.Begin();
    for (size_t i = 0; i < static_cast<size_t>(InputAction::_Count); ++i) {
        w.Write(ActionKey(static_cast<InputAction>(i)),
                SerializeBinding(m_bindings[i]));
    }
    w.End();

    std::ofstream f(path);
    if (f.is_open()) f << w.Str();
}

// --- Binding access --------------------------------------------------------

const InputBinding& InputMap::GetBinding(InputAction action) const {
    return m_bindings[static_cast<size_t>(action)];
}

void InputMap::SetBinding(InputAction action, InputBinding binding) {
    m_bindings[static_cast<size_t>(action)] = std::move(binding);
}

// --- Query API -------------------------------------------------------------

// Returns true when all required modifiers are held.
static bool ModifiersMatch(const app::InputBinding& b) {
    const ImGuiIO& io = ImGui::GetIO();
    if (b.requireCtrl  && !io.KeyCtrl)  return false;
    if (b.requireShift && !io.KeyShift) return false;
    if (b.requireAlt   && !io.KeyAlt)   return false;
    return true;
}

bool InputMap::IsHeld(InputAction action) const {
    const auto& b = GetBinding(action);
    if (!ModifiersMatch(b)) return false;
    switch (b.type) {
        case BindingType::Key:         return ImGui::IsKeyDown(b.key);
        case BindingType::MouseButton: return ImGui::IsMouseDown(b.mouseButton);
        default:                       return false;
    }
}

bool InputMap::IsPressed(InputAction action) const {
    const auto& b = GetBinding(action);
    if (!ModifiersMatch(b)) return false;
    switch (b.type) {
        case BindingType::Key:         return ImGui::IsKeyPressed(b.key);
        case BindingType::MouseButton: return ImGui::IsMouseClicked(b.mouseButton);
        default:                       return false;
    }
}

float InputMap::GetScrollAxis(InputAction action) const {
    const auto& b = GetBinding(action);
    if (b.type != BindingType::MouseScroll) return 0.0f;
    if (!ModifiersMatch(b)) return 0.0f;
    return ImGui::GetIO().MouseWheel;
}

ImVec2 InputMap::GetDragDelta(InputAction action) const {
    const auto& b = GetBinding(action);
    if (b.type != BindingType::MouseDrag) return ImVec2(0, 0);
    if (!ModifiersMatch(b)) return ImVec2(0, 0);
    if (!ImGui::IsMouseDragging(b.mouseButton)) return ImVec2(0, 0);
    return ImGui::GetIO().MouseDelta;
}

// --- Capture / remap -------------------------------------------------------

void InputMap::BeginCapture() {
    m_capturing       = true;
    m_capture_ready   = false;
    m_captured_binding = InputBinding::None();
}

bool InputMap::PollCapture(InputBinding& out_binding) {
    if (!m_capturing) return false;
    TickCapture();
    if (m_capture_ready) {
        out_binding  = m_captured_binding;
        m_capturing  = false;
        return true;
    }
    return false;
}

void InputMap::TickCapture() {
    // Priority: keyboard -> mouse buttons -> scroll -> drag.
    // Current modifier state is baked into the captured binding automatically.
    ImGuiIO& io = ImGui::GetIO();

    auto BakeModifiers = [](app::InputBinding b) -> app::InputBinding {
        const ImGuiIO& io2 = ImGui::GetIO();
        return b.WithModifiers(io2.KeyCtrl, io2.KeyShift, io2.KeyAlt);
    };

    for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
        auto key = static_cast<ImGuiKey>(k);
        if (key == ImGuiKey_LeftCtrl  || key == ImGuiKey_RightCtrl  ||
            key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift ||
            key == ImGuiKey_LeftAlt   || key == ImGuiKey_RightAlt)
            continue;
        if (ImGui::IsKeyPressed(key)) {
            m_captured_binding = BakeModifiers(app::InputBinding::FromKey(key));
            m_capture_ready    = true;
            return;
        }
    }

    for (int mb = 0; mb < 5; ++mb) {
        if (ImGui::IsMouseClicked(static_cast<ImGuiMouseButton>(mb))) {
            m_captured_binding = BakeModifiers(app::InputBinding::FromMouseButton(static_cast<ImGuiMouseButton>(mb)));
            m_capture_ready    = true;
            return;
        }
    }

    if (io.MouseWheel != 0.0f) {
        m_captured_binding = app::InputBinding::FromScroll(); // no modifiers baked for scroll
        m_capture_ready    = true;
        return;
    }

    for (int mb = 0; mb < 5; ++mb) {
        if (ImGui::IsMouseDragging(static_cast<ImGuiMouseButton>(mb), 4.0f)) {
            m_captured_binding = BakeModifiers(app::InputBinding::FromDrag(static_cast<ImGuiMouseButton>(mb)));
            m_capture_ready    = true;
            return;
        }
    }
}

} // namespace app
