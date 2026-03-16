#include "Network.h"

namespace app {
    Network* Network::instance = nullptr;

    Network::Network() {
        instance = this;

        if (!SteamAPI_Init()) {
            std::cerr << "SteamAPI failed to init! Is Steam running?" << std::endl;
            return;
        }
        std::cout << "Steam Connected as: " << SteamFriends()->GetPersonaName() << std::endl;
    }

    Network::~Network() {
        SteamAPI_Shutdown();

        instance = nullptr;
    }

    void Network::Update() {
        SteamAPI_RunCallbacks(); // This triggers the OnLobbyCreated etc. functions
    }

    void Network::HostLobby() {
        // k_ELobbyTypePublic: Anyone can see it. 
        // k_ELobbyTypeFriendsOnly: Only friends can join.
        SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 2);
        m_LobbyCreatedCallResult.Set(hSteamAPICall, this, &Network::OnLobbyCreated);
    }

    void Network::RefreshUserList()
    {
        if (!m_session.active) return;

        m_session.connectedUsers.clear();
        int count = SteamMatchmaking()->GetNumLobbyMembers(m_currentLobbyID);

        for (int i = 0; i < count; ++i) {
            CSteamID user = SteamMatchmaking()->GetLobbyMemberByIndex(m_currentLobbyID, i);

            UserData newdata;
            newdata.displayname = SteamFriends()->GetFriendPersonaName(user);

            m_session.connectedUsers.push_back(newdata);
        }
    }

    void Network::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure) {
        if (pCallback->m_eResult != k_EResultOK || bIOFailure) {
            std::cerr << "Failed to create lobby." << std::endl;
            return;
        }
        m_currentLobbyID = pCallback->m_ulSteamIDLobby;

        std::cout << "Lobby Created! ID: " << m_currentLobbyID.ConvertToUint64() << std::endl;
    }

    void Network::OnLobbyMemberStatusChange(LobbyChatUpdate_t* pCallback)
    {
        RefreshUserList();
    }

    void Network::JoinLobby(std::string lobbyIDStr) {
        uint64 id = std::stoull(lobbyIDStr);
        CSteamID lobbyID(id);
        SteamMatchmaking()->JoinLobby(lobbyID);
    }

    void Network::LeaveLobby()
    {
        m_session.active = false;
        SteamMatchmaking()->LeaveLobby(m_currentLobbyID);
    }

    void Network::SendChat(const std::string& message)
    {
        if (!m_session.active) return;

        // Steam handles the "Send" – it's limited to 4KB per message
        SteamMatchmaking()->SendLobbyChatMsg(m_currentLobbyID, message.c_str(), (int)message.length() + 1);
    }

    void Network::OnLobbyEntered(LobbyEnter_t* pCallback) {
        m_currentLobbyID = pCallback->m_ulSteamIDLobby;
        std::cout << "Joined Lobby successfully!" << std::endl;
        m_session.isHost = (SteamMatchmaking()->GetLobbyOwner(m_session.lobbyID) == SteamUser()->GetSteamID());

        m_session.active = true;

        RefreshUserList();
    }

    void Network::OnLobbyChatMsg(LobbyChatMsg_t* pCallback) {
        CSteamID speaker;
        static char buffer[4096]; // Max Steam chat size
        EChatEntryType type;

        int ret = SteamMatchmaking()->GetLobbyChatEntry(
            m_currentLobbyID,
            (int)pCallback->m_iChatID,
            &speaker,
            buffer,
            sizeof(buffer),
            &type
        );

        if (ret > 0 && type == k_EChatEntryTypeChatMsg) {
            ChatMessage msg;
            msg.sender = SteamFriends()->GetFriendPersonaName(speaker);
            msg.text = std::string(buffer);

            m_session.chatLog.push_back(msg);

            // Keep the log manageable (optional)
            if (m_session.chatLog.size() > 50) {
                m_session.chatLog.erase(m_session.chatLog.begin());
            }
        }
    }

    std::string Network::GetCurrentLobbyID() {
        return std::to_string(m_currentLobbyID.ConvertToUint64());
    }

}