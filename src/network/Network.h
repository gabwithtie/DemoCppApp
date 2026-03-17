#pragma once

#include <gab-steam/public/steam_api.h>
#include <string>
#include <iostream>
#include <vector>

#include "NetworkObject.h"

namespace app {
    struct UserData {
        std::string displayname;
    };
    struct ChatMessage {
        std::string sender;
        std::string text;
    };
    struct SessionData {
        bool active = false;
        uint64 lobbyID = 0;
        std::vector<UserData> connectedUsers;
        std::vector<ChatMessage> chatLog;
    };
    

    class Network {
    public:
        static inline Network* GetInstance() {
            return instance;
        }

        Network();
        ~Network();

        // Host a lobby (Public)
        void HostLobby();
        void BroadcastChanges();
        void SendSyncToAll(NETWORKREQUESTPARAMS);

        // Lobby actions
        void JoinLobby(std::string lobbyIDStr);
        void LeaveLobby();
        void SendChat(const std::string& message);

        // Must be called every frame in your game loop!
        void Update();
        void SendRequestPacket(NETWORKREQUESTPARAMS);

        // Get the current Lobby ID to show in ImGui
        std::string GetCurrentLobbyID();
        const SessionData& GetSessionData() const { return m_session; }

        bool IsHost() const {
            if (!m_session.active || !m_currentLobbyID.IsValid())
                return false;

            // Check if the current user is the owner of the lobby
            return SteamMatchmaking()->GetLobbyOwner(m_currentLobbyID) == SteamUser()->GetSteamID();
        }

    private:
        static Network* instance;
        
        CSteamID m_currentLobbyID;
        SessionData m_session;

        void RefreshUserList();

        void HandleIncomingMessages();
        void HandleInternalRequest(NETWORKREQUESTPARAMS);

        // Steam Callbacks
        void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
        CCallResult<Network, LobbyCreated_t> m_LobbyCreatedCallResult;

        STEAM_CALLBACK(Network, OnLobbyEntered, LobbyEnter_t);
        STEAM_CALLBACK(Network, OnLobbyChatMsg, LobbyChatMsg_t);
        STEAM_CALLBACK(Network, OnLobbyMemberStatusChange, LobbyChatUpdate_t);
        STEAM_CALLBACK(Network, OnSessionRequest, SteamNetworkingMessagesSessionRequest_t);
    };
}