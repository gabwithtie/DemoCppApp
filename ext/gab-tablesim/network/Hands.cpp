#include "Hands.h"

namespace app::gab {
	Hands::Hands() : NetworkDictionary(HANDS_REGISTRYID)
	{

	}

    void Hands::RemoveOne(std::string cardname)
    {
        // 1. Get a reference to the local player's hand
        HandObject myHand = instance->GetData();

        // 2. Find the card in the vector
        auto it = std::find(myHand.cards.begin(), myHand.cards.end(), cardname);

        // 3. If the card was found, remove it
        if (it != myHand.cards.end()) {
            myHand.cards.erase(it);

            // 4. IMPORTANT: Sync the updated hand to the network
            instance->RequestUpdate(myHand);
        }
    }

	std::vector<uint8_t> HandObject::Serialize()
    {
        std::vector<uint8_t> buffer;

        uint32_t count = (uint32_t)cards.size();
        uint8_t* pCount = (uint8_t*)&count;
        buffer.insert(buffer.end(), pCount, pCount + sizeof(count));

        for (const auto& s : cards) {
            uint16_t len = (uint16_t)s.size();
            uint8_t* pLen = (uint8_t*)&len;
            buffer.insert(buffer.end(), pLen, pLen + sizeof(len));
            buffer.insert(buffer.end(), s.begin(), s.end());
        }
        return buffer;
    }

    void HandObject::Deserialize(const uint8_t* data, size_t size) {
        size_t offset = 0;
        auto can_read = [&](size_t bytes) { return (offset + bytes) <= size; };

        // 1. Get Count
        if (!can_read(sizeof(uint32_t))) return;
        uint32_t count;
        memcpy(&count, data + offset, sizeof(count));
        offset += sizeof(count);

        // 2. Extract Strings
        cards.clear();
        for (uint32_t i = 0; i < count; i++) {
            if (!can_read(sizeof(uint16_t))) break;
            uint16_t len;
            memcpy(&len, data + offset, sizeof(len));
            offset += sizeof(len);

            if (!can_read(len)) break;
            cards.push_back(std::string((const char*)data + offset, len));
            offset += len;
        }
    }
}
