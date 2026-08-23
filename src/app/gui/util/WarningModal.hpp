#pragma once
#include <imgui.h>
#include <string>

namespace gsr {

class WarningModal {
public:
    static void Trigger(const std::string& message) {
        s_message = message;
        s_open_requested = true;
    }

    static void Render() {
        if (s_open_requested) {
            ImGui::OpenPopup("Warning##ModalWindow");
            s_open_requested = false;
        }

        if (ImGui::BeginPopupModal("Warning##ModalWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted(s_message.c_str());
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

private:
    inline static std::string s_message;
    inline static bool s_open_requested{false};
};

} // namespace gsr