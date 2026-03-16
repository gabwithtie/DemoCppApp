
#include "ChatWindow.h"

#include "network/Network.h"

namespace app {
	void ChatWindow::DrawSelf()
	{
        auto& session = Network::GetInstance()->GetSessionData();
        if (!session.active) return;

        ImGui::SeparatorText("Lobby Chat");

        // 1. Chat History Area
        float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_Border)) {
            for (const auto& msg : session.chatLog) {
                ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[%s]: ", msg.sender.c_str());
                ImGui::SameLine();
                ImGui::TextWrapped("%s", msg.text.c_str());
            }

            // Auto-scroll to bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        // 2. Message Input
        static char chatBuf[256] = "";
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        if (ImGui::InputText("##ChatInput", chatBuf, IM_ARRAYSIZE(chatBuf), input_flags)) {
            if (chatBuf[0] != '\0') {
                Network::GetInstance()->SendChat(chatBuf);
                chatBuf[0] = '\0'; // Clear buffer
                reclaim_focus = true;
            }
        }

        // Keep focus on the input box after pressing enter
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1);
	}
}