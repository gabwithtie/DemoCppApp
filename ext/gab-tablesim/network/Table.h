#pragma once

#include "network/NetworkObject.h"
#include "math/gbe_math.h"


#define TABLE_REGISTRYID 100

namespace app::gab {
	struct TableObject {
		uint32_t id;
		gbe::Vector2 position;
	};

	class Table : public NetworkObject<TableObject> {
	public:
		Table();

		void CreateObject(gbe::Vector2 position);
	};
}