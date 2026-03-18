#pragma once
#include <unordered_map>
#include "NetworkObject.h"
#include "Network.h" // Required for Network::GetInstance()->UserId()

namespace app {

    template<typename T>
    class NetworkDictionary : public INetworkObject {
        typedef std::pair<uint64_t, T> TPair;

    protected:
        static NetworkDictionary* instance;
    public:
        static NetworkDictionary* Get_instance() {
            return instance;
        }

        NetworkDictionary(uint16_t id) : m_id(id) {
            NetworkRegistry::Register(this);
            instance = this;
        }

        // Returns the data specifically for the LOCAL user
        T GetData() {
            uint64_t myId = Network::GetInstance()->UserId();

            return FindData(myId);
        }

        T& FindData(uint64_t myId) {
            // You now have to search for your ID
            for (auto& entry : m_map) {
                if (entry.first == myId) return entry.second;
            }

            // Lazy Init: If not found, add yourself to the list
            T defaultItem{};
            m_map.push_back({ myId, defaultItem });

            // Tell the host to add/update this entry
            // We use the UserId as the index for the network logic
            RequestUpdate(defaultItem);

            return m_map.back().second;
        }

        // Returns the whole map (useful for the UI to draw everyone's state)
        const std::unordered_map<uint64_t, T>& GetFullMap() const {
            return m_map;
        }

        // Send an update for the local user's data to the host
        void RequestUpdate(T updated) {
            uint64_t myId = Network::GetInstance()->UserId();
            auto data = updated.Serialize();
            callback_func(GetID(), NetActionType::Change, myId, data.data(), data.size() * sizeof(uint8_t));
        }

        void HardReset(void* data, size_t data_size, uint32_t element_count) override {
            m_map.clear();
            m_changes.clear();
            if (element_count > 0) {
                TPair* items = static_cast<TPair*>(data);
                m_map.assign(items, items + element_count);
            }
        }

        // --- INetworkObject Interface ---

        uint16_t GetID() const override { return m_id; }
        bool HasChanges() const override { return !m_changes.empty(); }
        void ClearChanges() override { m_changes.clear(); }

        void ApplyNetworkAction(NETWORKREQUESTPARAMS) override {
            // In a dictionary, action_index is treated as the UserID
            uint64_t userId = (uint64_t)action_index;
            T item = {};

            // 4. UPDATE: Handle incoming serialized data
            if constexpr (std::is_base_of<NetworkData, T>::value) {
                // Ensure your T has a way to rebuild itself from raw bytes
                item.Deserialize(static_cast<uint8_t*>(action_data), data_size);
            }
            else {
                memcpy(&item, action_data, sizeof(T));
            }

            FindData(userId) = item;

            PendingAction pa{ NetActionType::Change, userId };

            if constexpr (std::is_base_of<NetworkData, T>::value) {
                pa.data = item.Serialize(); // Use your new function
            }
            else {
                pa.data.resize(sizeof(T));
                memcpy(pa.data.data(), &item, sizeof(T));
            }
            pa.size = pa.data.size();

            m_changes.push_back(pa);
        }

        // Used by the Host to broadcast the whole dictionary to new players
        int GetElementCount() override { return (int)m_map.size(); }
        int GetElementSize() override { return sizeof(TPair); }

        // Note: For Dictionary, GetRawDataPtr is tricky because map is not contiguous.
        void* GetRawDataPtr() override {
            return m_map.data();
        }

    protected:
        uint16_t m_id;
        std::vector<TPair> m_map;
    };

    template<typename T>
    NetworkDictionary<T>* NetworkDictionary<T>::instance = nullptr;
}