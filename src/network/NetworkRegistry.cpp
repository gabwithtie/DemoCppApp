#include "NetworkRegistry.h"

#include "NetworkObject.h"

namespace app {
	void NetworkRegistry::Register(INetworkObject* obj)
	{
		GetMap()[obj->GetID()] = obj;
	}
	INetworkObject* NetworkRegistry::Get(uint16_t id)
	{
		if (GetMap().find(id) != GetMap().end())
			return GetMap()[id];
		return nullptr;
	}
	std::unordered_map<uint16_t, INetworkObject*>& NetworkRegistry::GetMap()
	{
		static std::unordered_map<uint16_t, INetworkObject*> instance;
		return instance;
	}
}


