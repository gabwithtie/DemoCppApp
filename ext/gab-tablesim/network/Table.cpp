#include "Table.h"

namespace app::gab {
	std::ostream& operator<<(std::ostream& os, const TableObject& s) {
		os << "Id: " << s.id << ", X: " << s.position.x << ", Y: " << s.position.y;
		return os;
	}
	
	Table::Table() : NetworkObject<TableObject>(TABLE_REGISTRYID) {

	}
	void Table::CreateObject(gbe::Vector2 position)
	{
		this->RequestAction(app::NetActionType::Add, 0, {
			.position = position
			});
	}
}