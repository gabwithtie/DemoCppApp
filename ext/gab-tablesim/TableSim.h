#pragma once

#include "network/Table.h"

namespace app::gab {
	class TableSim {
		Table table;
	public:
		TableSim();

		inline Table& Get_table() {
			return table;
		}
	};
}