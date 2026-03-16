#pragma once

#include <gab-steam/public/steam_api.h>
#include <string>
#include <iostream>
#include <vector>

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
        bool isHost = false;
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

        // Lobby actions
        void JoinLobby(std::string lobbyIDStr);
        void LeaveLobby();
        void SendChat(const std::string& message);

        // Must be called every frame in your game loop!
        void Update();

        // Get the current Lobby ID to show in ImGui
        std::string GetCurrentLobbyID();
        const SessionData& GetSessionData() const { return m_session; }

    private:
        static Network* instance;
        
        CSteamID m_currentLobbyID;
        SessionData m_session;

        void RefreshUserList();

        // Steam Callbacks
        void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure);
        CCallResult<Network, LobbyCreated_t> m_LobbyCreatedCallResult;

        STEAM_CALLBACK(Network, OnLobbyEntered, LobbyEnter_t);
        STEAM_CALLBACK(Network, OnLobbyChatMsg, LobbyChatMsg_t);
        STEAM_CALLBACK(Network, OnLobbyMemberStatusChange, LobbyChatUpdate_t);
    };
}