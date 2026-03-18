#pragma once
#include <vector>
#include <stdint.h>
#include <iostream>
#include <functional>
#include <string>

#include "NetworkRegistry.h"

namespace app {

    // The types of actions that can happen to a list
    enum class NetActionType : uint8_t {
        Add = 0,
        Remove = 1,
        Change = 2,
        Resync = 3
    };

    // Represents a single change to be synchronized
    struct PendingAction {
        NetActionType type;
        uint64_t index;
        std::vector<uint8_t> data;
        size_t size;
    };

    struct NetworkData {
        virtual std::vector<uint8_t> Serialize() = 0;
        virtual void Deserialize(const uint8_t* data, size_t size) = 0;
    };

    #define NETWORKREQUESTPARAMS uint16_t object_id, NetActionType action_type, uint64_t action_index, void* action_data, size_t data_size
    #define NETWORKREQUESTPARAMNAMES object_id, action_type, action_index, action_data, data_size
    typedef std::function<void(NETWORKREQUESTPARAMS)> NetworkRequestCallback_t;

    // Interface so we can store different templated types in one registry
    class INetworkObject {
    protected:
        static NetworkRequestCallback_t callback_func;
        std::vector<PendingAction> m_changes;
    public:

        const std::vector<PendingAction>& GetChanges() const { return m_changes; }

        static inline void SetCallback(NetworkRequestCallback_t _callback_func) {
            callback_func = _callback_func;
        }

        virtual ~INetworkObject() = default;
        virtual uint16_t GetID() const = 0;
        virtual bool HasChanges() const = 0;
        virtual void HardReset(void* data, size_t data_size, uint32_t element_count) = 0;
        virtual void ClearChanges() = 0;
        virtual int GetElementCount() = 0;
        virtual int GetElementSize() = 0;
        virtual void* GetRawDataPtr() = 0;

        virtual void ApplyNetworkAction(NETWORKREQUESTPARAMS) = 0;
    };


    template<typename T>
    class NetworkObject : public INetworkObject {
    protected:
        static NetworkObject* instance;
    public:
        static NetworkObject* Get_instance() {
            return instance;
        }

        NetworkObject(uint16_t id) : m_id(id) {
            instance = this;
            NetworkRegistry::Register(this);
        }

        // --- LOGIC FUNCTIONS ---

        // Host calls this to execute and broadcast
        void Add(T item) {
            m_data.push_back(item);

            PendingAction pa{ NetActionType::Add, (uint64_t)m_data.size() - 1 };
            if constexpr (std::is_base_of<NetworkData, T>::value) {
                pa.data = item.Serialize();
            }
            else {
                pa.data.resize(sizeof(T));
                memcpy(pa.data.data(), &item, sizeof(T));
            }
            pa.size = pa.data.size();

            m_changes.push_back(pa);
        }

        void Remove(uint64_t index) {
            if (index >= 0 && index < (uint64_t)m_data.size()) {
                m_data.erase(m_data.begin() + index);
                m_changes.push_back({ NetActionType::Remove, index, {}, 0 });
            }
        }

        void Change(uint64_t index, T item) {
            if (index >= 0 && index < (uint64_t)m_data.size()) {
                m_data[index] = item;

                PendingAction pa{ NetActionType::Change, index };
                if constexpr (std::is_base_of<NetworkData, T>::value) {
                    pa.data = item.Serialize();
                }
                else {
                    pa.data.resize(sizeof(T));
                    memcpy(pa.data.data(), &item, sizeof(T));
                }
                pa.size = pa.data.size();

                m_changes.push_back(pa);
            }
        }

        // Inside NetworkObject<T> implementation
        void HardReset(void* data, size_t data_size, uint32_t element_count) override {
            m_data.clear();
            m_changes.clear();
            if (element_count > 0) {
                T* items = static_cast<T*>(data);
                m_data.assign(items, items + element_count);
            }
        }

        // Client calls this to ask the host to do something
        // (Actual implementation of SendRequest depends on your Network class)
        void RequestAction(NetActionType type, int32_t index, T item = T{}) {
            auto data = item.Serialize();
            callback_func(GetID(), type, index, data.data(), data.size() * sizeof(uint8_t));
        }

        // --- SYNC INTERFACE ---

        uint16_t GetID() const override { return m_id; }
        bool HasChanges() const override { return !m_changes.empty(); }
        void ClearChanges() override { m_changes.clear(); }
        void ApplyNetworkAction(NETWORKREQUESTPARAMS) override {
            T item = {};

            // 4. UPDATE: Handle incoming serialized data
            if constexpr (std::is_base_of<NetworkData, T>::value) {
                // Ensure your T has a way to rebuild itself from raw bytes
                item.Deserialize(static_cast<uint8_t*>(action_data), data_size);
            }
            else {
                memcpy(&item, action_data, sizeof(T));
            }

            switch (action_type) {
            case NetActionType::Add:    this->Add(item);    break;
            case NetActionType::Remove: this->Remove(action_index); break;
            case NetActionType::Change: this->Change(action_index, item); break;
            }
        }
        int GetElementCount() override {
            return m_data.size();
        }
        int GetElementSize() override {
            return sizeof(T);
        }
        void* GetRawDataPtr() override {
            return m_data.data();
        }

        const std::vector<T>& GetData() const { return m_data; }

    protected:
        uint16_t m_id;
        std::vector<T> m_data;
    };

    template<typename T>
    NetworkObject<T>* NetworkObject<T>::instance = nullptr;
}