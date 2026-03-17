#pragma once

#include "network/NetworkObject.h"
#include "math/gbe_math.h"

#include <sstream>


#define TABLE_REGISTRYID 100

namespace app::gab {
	struct TableObject {
		uint32_t id;
		gbe::Vector2 position;
	};

	extern std::ostream& operator<<(std::ostream& os, const TableObject& s);

	class Table : public NetworkObject<TableObject> {
	public:
		Table();

		void CreateObject(gbe::Vector2 position);
	};
}