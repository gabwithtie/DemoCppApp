#pragma once

#include "network/Table.h"
#include "network/Decks.h"
#include "network/Hands.h"

namespace app::gab {
	class TableSim {
		Table table;
		Decks decks;
		Hands hands;

	public:
		TableSim();
	};
}