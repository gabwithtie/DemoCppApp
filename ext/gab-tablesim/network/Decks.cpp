#include "Decks.h"
#include "Decks.h"

namespace app::gab {

	Decks::Decks() : NetworkObject<DeckObject>(DECKS_REGISTRYID) {

	}
	void Decks::CreateObject(std::string name, std::vector<std::string> cardnames)
	{
		auto newobj = DeckObject();
		newobj.id = instance->GetData().size();
		newobj.name = name;
		newobj.cardnames = cardnames;

		static_cast<Decks*>(instance)->RequestAction(app::NetActionType::Add, 0, newobj);
	}

	std::vector<uint8_t> DeckObject::Serialize()
    {
        std::vector<uint8_t> buffer;

        // ID
        uint8_t* pId = (uint8_t*)&id;
        buffer.insert(buffer.end(), pId, pId + sizeof(id));

        // Name (Length + Chars)
        uint16_t nameLen = (uint16_t)name.size();
        uint8_t* pNLen = (uint8_t*)&nameLen;
        buffer.insert(buffer.end(), pNLen, pNLen + sizeof(nameLen));
        buffer.insert(buffer.end(), name.begin(), name.end());

        // Card Vector Count
        uint32_t count = (uint32_t)cardnames.size();
        uint8_t* pCount = (uint8_t*)&count;
        buffer.insert(buffer.end(), pCount, pCount + sizeof(count));

        // Individual Cards
        for (const auto& s : cardnames) {
            uint16_t len = (uint16_t)s.size();
            uint8_t* pLen = (uint8_t*)&len;
            buffer.insert(buffer.end(), pLen, pLen + sizeof(len));
            buffer.insert(buffer.end(), s.begin(), s.end());
        }
        return buffer;
    }
    void app::gab::DeckObject::Deserialize(const uint8_t* data, size_t size)
    {
        size_t offset = 0;

        // Helper to check if we can safely read N bytes
        auto can_read = [&](size_t bytes) {
            return (offset + bytes) <= size;
            };

        // 1. Unpack ID
        if (!can_read(sizeof(id))) return;
        memcpy(&id, data + offset, sizeof(id));
        offset += sizeof(id);

        // 2. Unpack Name
        if (!can_read(sizeof(uint16_t))) return;
        uint16_t nameLen;
        memcpy(&nameLen, data + offset, sizeof(nameLen));
        offset += sizeof(nameLen);

        if (nameLen > 0) {
            if (!can_read(nameLen)) return; // CRASH PROTECTION
            name = std::string((const char*)data + offset, nameLen);
            offset += nameLen;
        }

        // 3. Unpack Card Count
        if (!can_read(sizeof(uint32_t))) return;
        uint32_t count;
        memcpy(&count, data + offset, sizeof(count));
        offset += sizeof(count);

        cardnames.clear();
        for (uint32_t i = 0; i < count; i++) {
            if (!can_read(sizeof(uint16_t))) break;
            uint16_t len;
            memcpy(&len, data + offset, sizeof(len));
            offset += sizeof(len);

            if (!can_read(len)) break; // CRASH PROTECTION
            cardnames.push_back(std::string((const char*)data + offset, len));
            offset += len;
        }
    }
}