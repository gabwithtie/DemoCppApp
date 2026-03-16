#include "NetworkWindow.h"

#include "network/Network.h"

namespace app {
    // Enum to manage the UI state
    enum class NetworkState { Idle, Hosting, Joining };
    static NetworkState currentState = NetworkState::Idle;
    static char joinBuffer[128] = "";

    void NetworkWindow::DrawSelf()
    {
        auto net = Network::GetInstance();

        // STATE 1: IDLE (Initial View)
        if (currentState == NetworkState::Idle) {
            if (ImGui::Button("Host Game", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                net->HostLobby();
                currentState = NetworkState::Hosting;
            }

            if (ImGui::Button("Join Game", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                ImGui::OpenPopup("Join Lobby");
            }

            // JOIN POPUP
            if (ImGui::BeginPopupModal("Join Lobby", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Enter the Lobby ID provided by the host:");
                ImGui::InputText("##id_input", joinBuffer, IM_ARRAYSIZE(joinBuffer));

                if (ImGui::Button("Connect")) {
                    if (strlen(joinBuffer) > 0) {
                        net->JoinLobby(joinBuffer);
                        currentState = NetworkState::Joining;
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }
        }

        // STATE 2: HOSTING or JOINING (Active Session View)
        else {
            std::string lobbyID = net->GetCurrentLobbyID();

            ImGui::TextDisabled("Session ID:");
            ImGui::Text("%s", lobbyID.c_str());

            ImGui::SameLine();
            if (ImGui::Button("Copy ID")) {
                ImGui::SetClipboardText(lobbyID.c_str());
            }

            ImGui::Separator();
            ImGui::Text("Connected Users:");

            // Display Lobby Members
            // Note: We use SteamMatchmaking() directly to list users
            for (const auto& userdata : net->GetSessionData().connectedUsers)
            {
                ImGui::BulletText("%s", userdata.displayname.c_str());
            }

            ImGui::Dummy(ImVec2(0, 20)); // Custom spacer or use ImGui::Dummy

            // LEAVE / END BUTTONS
            const char* leaveButtonText = (currentState == NetworkState::Hosting) ? "End Host" : "Leave Game";
            if (ImGui::Button(leaveButtonText, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                if (currentState == NetworkState::Hosting) {
                    // Logic to actually close lobby in Steam
                    net->LeaveLobby();
                }
                else {
                    net->LeaveLobby();
                }
                currentState = NetworkState::Idle;
            }
        }
    }
}