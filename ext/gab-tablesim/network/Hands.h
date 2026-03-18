#pragma once

#include <string>

#include "network/NetworkDictionary.h"

#define HANDS_REGISTRYID 102

namespace app::gab {

    struct HandObject : NetworkData {
        std::vector<std::string> cards;

        std::vector<uint8_t> Serialize() override;
        void Deserialize(const uint8_t* data, size_t size) override;
    };

	class Hands : public NetworkDictionary<HandObject> {
	public:
		Hands();
        static void RemoveOne(std::string cardname);
    };
}