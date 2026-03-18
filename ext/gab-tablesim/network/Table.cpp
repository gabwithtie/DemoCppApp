#include "Table.h"
#include "Table.h"
#include "Table.h"

namespace app::gab {
    std::vector<uint8_t> TableObject::Serialize()
    {
        std::vector<uint8_t> buffer;
        // 1. Pack fixed members individually (Avoids VTable/Padding issues)
        auto pack = [&](const auto& val) {
            uint8_t const* p = reinterpret_cast<uint8_t const*>(&val);
            buffer.insert(buffer.end(), p, p + sizeof(val));
            };

        pack(id);
        pack(position);
        pack(object_type);
        pack(face_up);

        // 2. Pack vector size
        uint32_t count = (uint32_t)cardshere.size();
        pack(count);

        // 3. Pack strings
        for (const auto& s : cardshere) {
            uint16_t len = (uint16_t)s.size();
            pack(len);
            buffer.insert(buffer.end(), s.begin(), s.end());
        }
        return buffer;
    }

    void TableObject::Deserialize(const uint8_t* data, size_t size) {
        size_t offset = 0;

        // Safety helper: returns false if we'd read past the end of the packet
        auto can_read = [&](size_t bytes) { return (offset + bytes) <= size; };

        // 1. Fixed Members
        if (!can_read(sizeof(id))) return;
        memcpy(&id, data + offset, sizeof(id));
        offset += sizeof(id);

        if (!can_read(sizeof(position))) return;
        memcpy(&position, data + offset, sizeof(position));
        offset += sizeof(position);

        if (!can_read(sizeof(object_type))) return;
        memcpy(&object_type, data + offset, sizeof(object_type));
        offset += sizeof(object_type);

        if (!can_read(sizeof(face_up))) return;
        memcpy(&face_up, data + offset, sizeof(face_up));
        offset += sizeof(face_up);

        // 2. Card Vector
        if (!can_read(sizeof(uint32_t))) return;
        uint32_t count;
        memcpy(&count, data + offset, sizeof(count));
        offset += sizeof(count);

        cardshere.clear();
        for (uint32_t i = 0; i < count; i++) {
            if (!can_read(sizeof(uint16_t))) break;
            uint16_t len;
            memcpy(&len, data + offset, sizeof(len));
            offset += sizeof(len);

            if (!can_read(len)) break; // Stop if the string data is missing
            cardshere.push_back(std::string((const char*)data + offset, len));
            offset += len;
        }
    }

	Table::Table() : NetworkObject<TableObject>(TABLE_REGISTRYID) {

	}
	void Table::CreateObject(TableObject& _new)
	{
		instance->RequestAction(app::NetActionType::Add, 0, _new);
	}
    void app::gab::Table::DrawFrom(uint64_t index, TableObject& _new)
    {
        if(_new.cardshere.size() == 0)
            instance->RequestAction(NetActionType::Remove, index);
        else
            instance->RequestAction(NetActionType::Change, index, _new);
    }
}