#pragma once

#include "../../gui/main/GuiWindow.h"
#include "IllustratorWindow.hpp"
#include "tools/Tool.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace app {

class HotkeyWindow : public app::GuiWindow {
    public:
    explicit HotkeyWindow();
    
    std::string GetWindowId() override { return "Tool Hotkeys"; }
    
    void HandleHotkeys(IllustratorWindow& illustrator);
protected:
    void DrawSelf() override;

private:
    struct HotkeyBinding {
        Tool* tool{nullptr};
        std::string toolName;
        ImGuiKey key{ImGuiKey_None};
    };

    std::vector<std::unique_ptr<Tool>> ownedTools;
    std::vector<HotkeyBinding> bindings;
    std::filesystem::path configFilePath;

    std::string statusMessage;
    bool statusIsError{false};

    void ResetToDefaultBindings();
    bool SaveBindings() const;
    bool LoadBindings(bool allowMissingFile);

    static std::filesystem::path ResolveConfigFilePath();
    static const std::vector<std::pair<const char*, ImGuiKey>>& GetAssignableKeys();
    static const char* KeyToLabel(ImGuiKey key);
    static bool IsBoundKeyPressed(ImGuiKey key);
};

} // namespace app