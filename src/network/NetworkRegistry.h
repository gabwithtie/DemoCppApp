#pragma once

#include <unordered_map>

namespace app {
    class INetworkObject;

    class NetworkRegistry {
    public:
        static void Register(INetworkObject* obj);

        static INetworkObject* Get(uint16_t id);

        static std::unordered_map<uint16_t, INetworkObject*>& GetMap();
    };
}