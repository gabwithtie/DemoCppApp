#pragma once

#include "network/NetworkObject.h"
#include "math/gbe_math.h"

#include <sstream>


#define DECKS_REGISTRYID 101

namespace app::gab {
    struct DeckObject : NetworkData {
        uint32_t id;
        std::string name;
        std::vector<std::string> cardnames;

        std::vector<uint8_t> Serialize() override;

        void Deserialize(const uint8_t* data, size_t size) override;
    };

	class Decks : public NetworkObject<DeckObject> {
	public:
		Decks();

		static void CreateObject(std::string name, std::vector<std::string> cardnames);
	};
}