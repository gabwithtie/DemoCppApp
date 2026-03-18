#pragma once

#include "network/NetworkObject.h"
#include "math/gbe_math.h"

#include <sstream>


#define TABLE_REGISTRYID 100

namespace app::gab {
	enum TableObjectType {
		CARDS
	};

    struct TableObject : NetworkData {
        uint64_t id;
        gbe::Vector2 position;
        TableObjectType object_type;
		bool face_up = false;
        std::vector<std::string> cardshere;

		std::vector<uint8_t> Serialize() override;
        void Deserialize(const uint8_t* data, size_t size) override;
    };

	class Table : public NetworkObject<TableObject> {
	public:
		Table();

		static void CreateObject(TableObject& _new);
		static void DrawFrom(uint64_t index, TableObject& _new);
	};
}