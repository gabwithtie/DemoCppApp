#include "Table.h"

namespace app::gab {
	Table::Table() : NetworkObject<TableObject>(TABLE_REGISTRYID) {

	}
	void Table::CreateObject(gbe::Vector2 position)
	{
		this->RequestAction(app::NetActionType::Add, 0, {
			.position = position
			});
	}
}