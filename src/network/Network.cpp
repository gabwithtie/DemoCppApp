#include "Network.h"
#include "Network.h"
#include "Network.h"

namespace app {
#pragma pack(push, 1)
    struct RequestPacketHeader {
        uint8_t packetType = 3; // ID 3 = Client-to-Host Request
        uint16_t objectID;
        uint8_t actionType;
        int32_t index;
        // Followed by raw data of size 'data_size'
    };
#pragma pack(pop)
#pragma pack(push, 1)
    struct SyncPacketHeader {
        uint8_t packetType = 4; // ID 4 = Host-to-Client Sync
        uint16_t objectID;
        uint8_t actionType;
        int32_t index;
        // Followed by T item data
    };
#pragma pack(pop)
#pragma pack(push, 1)
    struct ResyncPacketHeader {
        uint8_t packetType = 5; // ID 5 = Host-to-Client Hard Resync
        uint16_t objectID;
        uint32_t elementCount;  // How many T items are in the payload
        // Followed by (elementCount * sizeof(T)) bytes
    };
#pragma pack(pop)

    Network* Network::instance = nullptr;

    Network::Network() {
        instance = this;
        static app::NetworkRequestCallback_t callback_func = [this](NETWORKREQUESTPARAMS) {
            this->SendRequestPacket(NETWORKREQUESTPARAMNAMES);
            };
        INetworkObject::SetCallback(callback_func);

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
        HandleIncomingMessages();

        // 1. Get all registered network objects (Table, etc.)
        auto& registryMap = NetworkRegistry::GetMap();

        for (auto const& [id, obj] : registryMap) {
            if (!obj->HasChanges()) continue;

            // Only the Host is responsible for broadcasting the state
            if (this->IsHost()) {
                auto& changes = obj->GetChanges();

                for (const auto& change : changes) {
                    SendSyncToAll(id, change.type, change.index, (void*)&change.data, change.size);
                }
            }

            obj->ClearChanges();
        }
    }

    void Network::SendSyncToAll(NETWORKREQUESTPARAMS) {
        // Prepare the buffer
        SyncPacketHeader header{ 4, object_id, (uint8_t)action_type, action_index};
        size_t totalSize = sizeof(SyncPacketHeader) + data_size;
        std::vector<uint8_t> buffer(totalSize);
        memcpy(buffer.data(), &header, sizeof(SyncPacketHeader));
        if (action_data && data_size > 0) {
            memcpy(buffer.data() + sizeof(SyncPacketHeader), action_data, data_size);
        }

        // Send to every member in the lobby except ourselves
        int numMembers = SteamMatchmaking()->GetNumLobbyMembers(m_currentLobbyID);
        for (int i = 0; i < numMembers; i++) {
            CSteamID memberID = SteamMatchmaking()->GetLobbyMemberByIndex(m_currentLobbyID, i);
            if (memberID == SteamUser()->GetSteamID()) continue;

            SteamNetworkingIdentity identity;
            identity.Clear();
            identity.SetSteamID64(memberID.ConvertToUint64());

            // Use Unreliable for high-frequency moves (Change), Reliable for Add/Remove
            auto sendType = (action_type == NetActionType::Change) ? k_nSteamNetworkingSend_Unreliable : k_nSteamNetworkingSend_Reliable;

            SteamNetworkingMessages()->SendMessageToUser(identity, buffer.data(), totalSize, sendType, 0);
        }
    }

    void Network::SendRequestPacket(NETWORKREQUESTPARAMS) {
        if (!m_session.active) return;

        // 1. Loopback for Host
        if (IsHost()) {
            this->HandleInternalRequest(NETWORKREQUESTPARAMNAMES);
            return;
        }

        // 2. Prepare Buffer
        RequestPacketHeader header = { 3, object_id, (uint8_t)action_type, action_index };

        size_t totalSize = sizeof(RequestPacketHeader) + data_size;
        std::vector<uint8_t> buffer(totalSize);
        memcpy(buffer.data(), &header, sizeof(RequestPacketHeader));
        if (action_data && data_size > 0) {
            memcpy(buffer.data() + sizeof(RequestPacketHeader), action_data, data_size);
        }

        // 3. Send via NetworkingMessages
        CSteamID hostID = SteamMatchmaking()->GetLobbyOwner(m_currentLobbyID);

        // Corrected Identity setup:
        SteamNetworkingIdentity identity = {};
        identity.Clear();
        identity.SetSteamID64(hostID.ConvertToUint64());

        auto sendresult = SteamNetworkingMessages()->SendMessageToUser(
            identity,
            buffer.data(),
            (uint32)totalSize,
            k_nSteamNetworkingSend_Reliable,
            0 // Channel
        );
    }

    void Network::HandleIncomingMessages() {
        SteamNetworkingMessage_t* pMessages[16];
        int numMessages = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, pMessages, 16);

        for (int i = 0; i < numMessages; i++) {
            SteamNetworkingMessage_t* pMsg = pMessages[i];
            uint8_t* rawData = (uint8_t*)pMsg->m_pData;

            // The first byte is our Packet Type
            uint8_t packetType = rawData[0];

            if (packetType == 3 && IsHost()) {
                // It's a Request from a Client, and I am the Host (the Referee)
                RequestPacketHeader* header = (RequestPacketHeader*)rawData;

                void* payload = (pMsg->m_cbSize > sizeof(RequestPacketHeader)) ? (rawData + sizeof(RequestPacketHeader)) : nullptr;
                size_t payloadSize = pMsg->m_cbSize - sizeof(RequestPacketHeader);

                if ((NetActionType)header->actionType == NetActionType::Resync) { // Our code for Resync Request
                    // Prepare the packet
                    auto obj = NetworkRegistry::Get(header->objectID);

                    ResyncPacketHeader rheader{ 5, header->objectID, obj->GetElementCount() };
                    size_t payloadSize = obj->GetElementCount() * obj->GetElementSize();

                    std::vector<uint8_t> buffer(sizeof(rheader) + payloadSize);
                    memcpy(buffer.data(), &rheader, sizeof(rheader));
                    if (payloadSize > 0) {
                        memcpy(buffer.data() + sizeof(rheader), obj->GetRawDataPtr(), payloadSize);
                    }

                    // Send ONLY to the requester (we need the requester's SteamID)
                    // Note: You'll need to pass the pMsg->m_identityRemote from HandleIncomingMessages 
                    // down into HandleInternalRequest to know who to reply to.
                    SteamNetworkingMessages()->SendMessageToUser(pMsg->m_identityPeer, buffer.data(), (uint32)buffer.size(), k_nSteamNetworkingSend_Reliable, 0);

                    return;
                }

                this->HandleInternalRequest(header->objectID, (NetActionType)header->actionType, header->index, payload, payloadSize);
            }
            else if (packetType == 4 && !this->IsHost()) {
                // It's a Sync from the Host to a client
                SyncPacketHeader* h = (SyncPacketHeader*)rawData;
                auto obj = NetworkRegistry::Get(h->objectID);
                if (obj) {
                    void* data = (rawData + sizeof(SyncPacketHeader));
                    size_t payloadSize = pMsg->m_cbSize - sizeof(SyncPacketHeader);

                    obj->ApplyNetworkAction(h->objectID, (NetActionType)h->actionType, h->index, data, payloadSize);
                }
            }
            else if (packetType == 5 && !IsHost()) {
                ResyncPacketHeader* h = (ResyncPacketHeader*)rawData;
                auto obj = NetworkRegistry::Get(h->objectID);
                if (obj) {
                    void* payload = (rawData + sizeof(ResyncPacketHeader));
                    size_t payloadSize = pMsg->m_cbSize - sizeof(ResyncPacketHeader);

                    obj->HardReset(payload, payloadSize, h->elementCount);
                    std::cout << "Object " << h->objectID << " hard resynced. Elements: " << h->elementCount << std::endl;
                }
            }

            pMsg->Release();
        }
    }

    void Network::HandleInternalRequest(NETWORKREQUESTPARAMS) {
        if (!IsHost()) return;

        // Find the Table (or any NetworkObject) by its ID
        auto obj = NetworkRegistry::Get(object_id);
        if (!obj) return;

        obj->ApplyNetworkAction(NETWORKREQUESTPARAMNAMES);
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

    void Network::OnSessionRequest(SteamNetworkingMessagesSessionRequest_t* pCallback) {
        // Automatically accept all connection requests from people in our lobby
        SteamNetworkingMessages()->AcceptSessionWithUser(pCallback->m_identityRemote);
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

    void Network::Resync()
    {
        if (IsHost()) return; // Host doesn't need to resync with itself

        auto& registryMap = NetworkRegistry::GetMap();

        for (auto const& [id, obj] : registryMap) {
            SendRequestPacket(id, NetActionType::Resync, -1, nullptr, 0);
        }
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