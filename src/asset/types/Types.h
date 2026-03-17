#pragma once

namespace app {
    enum AssetType {
        NONE,
        MESH,
        TEXTURE,
        MATERIAL,
        SHADER,
        AUDIO,
        OBJECT
    };

    // Helper to get the ImGui-internal string ID from the enum
    inline const char* GetTypeString(AssetType type) {
        switch (type) {
        case AssetType::TEXTURE:  return "ASSET_PAYLOAD_TEX";
        case AssetType::MESH:     return "ASSET_PAYLOAD_MESH";
        case AssetType::MATERIAL: return "ASSET_PAYLOAD_MAT";
        case AssetType::OBJECT:   return "ASSET_PAYLOAD_OBJECT";
        default:                  return "ASSET_PAYLOAD_UNKNOWN";
        }
    }
}