#include "HotkeyWindow.hpp"

#include "tools/ToolRegistry.hpp"

#include <algorithm>
#include <fstream>
#include <imgui.h>
#include <sstream>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

std::string Trim(const std::string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

} // namespace

namespace app {

HotkeyWindow::HotkeyWindow(){
    ownedTools = ToolRegistry::CreateAll();
    configFilePath = ResolveConfigFilePath();

    ResetToDefaultBindings();
    LoadBindings(true);
}

#ifdef _WIN32
std::filesystem::path HotkeyWindow::ResolveConfigFilePath() {
    char exePath[MAX_PATH] = {};
    const DWORD copied = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (copied > 0) {
        return std::filesystem::path(exePath).parent_path() / "tool_hotkeys.ini";
    }

    return std::filesystem::current_path() / "tool_hotkeys.ini";
}
#else
std::filesystem::path HotkeyWindow::ResolveConfigFilePath() {
    return std::filesystem::current_path() / "tool_hotkeys.ini";
}
#endif

const std::vector<std::pair<const char*, ImGuiKey>>& HotkeyWindow::GetAssignableKeys() {
    static const std::vector<std::pair<const char*, ImGuiKey>> keys = {
        {"None", ImGuiKey_None},
        {"1", ImGuiKey_1},
        {"2", ImGuiKey_2},
        {"3", ImGuiKey_3},
        {"4", ImGuiKey_4},
        {"5", ImGuiKey_5},
        {"6", ImGuiKey_6},
        {"7", ImGuiKey_7},
        {"8", ImGuiKey_8},
        {"9", ImGuiKey_9},
        {"0", ImGuiKey_0},
        {"A", ImGuiKey_A},
        {"B", ImGuiKey_B},
        {"C", ImGuiKey_C},
        {"D", ImGuiKey_D},
        {"E", ImGuiKey_E},
        {"F", ImGuiKey_F},
        {"G", ImGuiKey_G},
        {"H", ImGuiKey_H},
        {"I", ImGuiKey_I},
        {"J", ImGuiKey_J},
        {"K", ImGuiKey_K},
        {"L", ImGuiKey_L},
        {"M", ImGuiKey_M},
        {"N", ImGuiKey_N},
        {"O", ImGuiKey_O},
        {"P", ImGuiKey_P},
        {"Q", ImGuiKey_Q},
        {"R", ImGuiKey_R},
        {"S", ImGuiKey_S},
        {"T", ImGuiKey_T},
        {"U", ImGuiKey_U},
        {"V", ImGuiKey_V},
        {"W", ImGuiKey_W},
        {"X", ImGuiKey_X},
        {"Y", ImGuiKey_Y},
        {"Z", ImGuiKey_Z},
        {"F1", ImGuiKey_F1},
        {"F2", ImGuiKey_F2},
        {"F3", ImGuiKey_F3},
        {"F4", ImGuiKey_F4},
        {"F5", ImGuiKey_F5},
        {"F6", ImGuiKey_F6},
        {"F7", ImGuiKey_F7},
        {"F8", ImGuiKey_F8},
        {"F9", ImGuiKey_F9},
        {"F10", ImGuiKey_F10},
        {"F11", ImGuiKey_F11},
        {"F12", ImGuiKey_F12},
        {"KP0", ImGuiKey_Keypad0},
        {"KP1", ImGuiKey_Keypad1},
        {"KP2", ImGuiKey_Keypad2},
        {"KP3", ImGuiKey_Keypad3},
        {"KP4", ImGuiKey_Keypad4},
        {"KP5", ImGuiKey_Keypad5},
        {"KP6", ImGuiKey_Keypad6},
        {"KP7", ImGuiKey_Keypad7},
        {"KP8", ImGuiKey_Keypad8},
        {"KP9", ImGuiKey_Keypad9}
    };

    return keys;
}

const char* HotkeyWindow::KeyToLabel(ImGuiKey key) {
    const auto& keys = GetAssignableKeys();
    for (const auto& keyPair : keys) {
        if (keyPair.second == key) {
            return keyPair.first;
        }
    }

    return "Unknown";
}

bool HotkeyWindow::IsBoundKeyPressed(ImGuiKey key) {
    if (key == ImGuiKey_None) {
        return false;
    }

    return ImGui::IsKeyPressed(key, false);
}

void HotkeyWindow::ResetToDefaultBindings() {
    static const ImGuiKey defaultOrder[] = {
        ImGuiKey_1,
        ImGuiKey_2,
        ImGuiKey_3,
        ImGuiKey_4,
        ImGuiKey_5,
        ImGuiKey_6,
        ImGuiKey_7,
        ImGuiKey_8,
        ImGuiKey_9,
        ImGuiKey_0
    };

    bindings.clear();
    bindings.reserve(ownedTools.size());

    for (size_t i = 0; i < ownedTools.size(); ++i) {
        Tool* tool = ownedTools[i].get();
        if (tool == nullptr) {
            continue;
        }

        const ImGuiKey key = (i < sizeof(defaultOrder) / sizeof(defaultOrder[0]))
            ? defaultOrder[i]
            : ImGuiKey_None;

        HotkeyBinding binding;
        binding.tool = tool;
        binding.toolName = tool->GetToolName();
        binding.key = key;
        bindings.push_back(binding);
    }
}

bool HotkeyWindow::SaveBindings() const {
    std::ofstream outFile(configFilePath, std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }

    outFile << "# Tool hotkey initialization file\n";
    outFile << "# Format: ToolName=Hotkey\n";
    for (const HotkeyBinding& binding : bindings) {
        outFile << binding.toolName << "=" << KeyToLabel(binding.key) << "\n";
    }

    return outFile.good();
}

bool HotkeyWindow::LoadBindings(bool allowMissingFile) {
    std::ifstream inFile(configFilePath);
    if (!inFile.is_open()) {
        return allowMissingFile;
    }

    std::unordered_map<std::string, ImGuiKey> keyByLabel;
    for (const auto& keyPair : GetAssignableKeys()) {
        keyByLabel.emplace(keyPair.first, keyPair.second);
    }

    std::string line;
    while (std::getline(inFile, line)) {
        const std::string trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }

        const size_t separatorPos = trimmed.find('=');
        if (separatorPos == std::string::npos) {
            continue;
        }

        const std::string toolName = Trim(trimmed.substr(0, separatorPos));
        const std::string keyName = Trim(trimmed.substr(separatorPos + 1));
        if (toolName.empty() || keyName.empty()) {
            continue;
        }

        const auto keyIt = keyByLabel.find(keyName);
        if (keyIt == keyByLabel.end()) {
            continue;
        }

        for (HotkeyBinding& binding : bindings) {
            if (binding.toolName == toolName) {
                binding.key = keyIt->second;
                break;
            }
        }
    }

    return true;
}

void HotkeyWindow::HandleHotkeys(IllustratorWindow& illustrator) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) {
        return;
    }

    for (const HotkeyBinding& binding : bindings) {
        if (binding.tool == nullptr || binding.key == ImGuiKey_None) {
            continue;
        }

        if (IsBoundKeyPressed(binding.key)) {
            illustrator.SetActiveTool(binding.tool);
            return;
        }
    }
}

void HotkeyWindow::DrawSelf() {
    if (ownedTools.empty()) {
        ImGui::TextUnformatted("No tools registered.");
        return;
    }

    ImGui::Text("Config File: %s", configFilePath.string().c_str());
    if (ImGui::Button("Save Hotkeys")) {
        if (SaveBindings()) {
            statusMessage = "Saved tool hotkeys.";
            statusIsError = false;
        } else {
            statusMessage = "Failed to save tool hotkeys.";
            statusIsError = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload From File")) {
        ResetToDefaultBindings();
        if (LoadBindings(false)) {
            statusMessage = "Reloaded tool hotkeys from file.";
            statusIsError = false;
        } else {
            statusMessage = "Hotkey file not found or not readable.";
            statusIsError = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Defaults")) {
        ResetToDefaultBindings();
        statusMessage = "Reset hotkeys to defaults.";
        statusIsError = false;
    }

    if (!statusMessage.empty()) {
        const ImVec4 color = statusIsError
            ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f)
            : ImVec4(0.5f, 1.0f, 0.5f, 1.0f);
        ImGui::TextColored(color, "%s", statusMessage.c_str());
    }

    bool hasDuplicateHotkeys = false;
    for (size_t i = 0; i < bindings.size(); ++i) {
        if (bindings[i].key == ImGuiKey_None) {
            continue;
        }
        for (size_t j = i + 1; j < bindings.size(); ++j) {
            if (bindings[i].key == bindings[j].key) {
                hasDuplicateHotkeys = true;
                break;
            }
        }
        if (hasDuplicateHotkeys) {
            break;
        }
    }

    if (hasDuplicateHotkeys) {
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f),
                           "Duplicate bindings detected. First matching tool wins.");
    }

    ImGui::TextUnformatted("Tool selection hotkeys:");
    ImGui::Separator();

    const auto& assignableKeys = GetAssignableKeys();
    if (ImGui::BeginTable("ToolHotkeyBindings", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Tool", ImGuiTableColumnFlags_WidthStretch, 0.65f);
        ImGui::TableSetupColumn("Hotkey", ImGuiTableColumnFlags_WidthStretch, 0.35f);
        ImGui::TableHeadersRow();

        for (HotkeyBinding& binding : bindings) {
            if (binding.tool == nullptr) {
                continue;
            }

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s%s", binding.toolName.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(binding.tool);
            ImGui::SetNextItemWidth(-1.0f);
            const char* previewLabel = KeyToLabel(binding.key);
            if (ImGui::BeginCombo("##hotkey", previewLabel)) {
                for (const auto& keyOption : assignableKeys) {
                    const bool selected = (binding.key == keyOption.second);
                    if (ImGui::Selectable(keyOption.first, selected)) {
                        binding.key = keyOption.second;
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

} // namespace app