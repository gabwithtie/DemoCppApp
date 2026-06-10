#include "AnimationManager.h"
#include <glaze/glaze.hpp>
#include <fstream>
#include <iostream>

namespace picsel {

    // Proxy struct to easily serialize base clip settings using Glaze
    struct ClipSaveData {
        std::string name;
        int width;
        int height;
        int fps;
    };
}

// Glaze reflection for the proxy struct
template <>
struct glz::meta<picsel::ClipSaveData> {
    using T = picsel::ClipSaveData;
    static constexpr auto value = object(
        "name", &T::name,
        "width", &T::width,
        "height", &T::height,
        "fps", &T::fps
    );
};

namespace picsel {

    std::shared_ptr<AnimationClip> AnimationManager::GetActiveClip() {
        return s_active_clip;
    }

    void AnimationManager::SetActiveClip(std::shared_ptr<AnimationClip> clip) {
        s_active_clip = clip;
    }

    std::string AnimationManager::GetActiveClipPath() {
        return s_active_clip_path.string();
    }

    bool AnimationManager::CreateNewClip(const std::filesystem::path& directory, const std::string& name, int width, int height) {
        std::filesystem::path filepath = directory / (name + ".anim");

        ClipSaveData new_data{ name, width, height, 12 };

        std::string json_buffer;
        glz::write_json(new_data, json_buffer);
        
        std::ofstream file(filepath);
        if (!file.is_open()) return false;
        file << json_buffer;
        file.close();

        // Automatically load it into the active state after creation
        return LoadClip(filepath);
    }

    bool AnimationManager::LoadClip(const std::filesystem::path& filepath) {
        if (!std::filesystem::exists(filepath)) return false;

        ClipSaveData loaded_data;
        std::string buffer;

        auto ec = glz::read_file_json < glz::opts{ .error_on_unknown_keys = false } > (loaded_data, filepath.string(), buffer);
        if (ec) {
            std::cerr << "[Picsel] Failed to load animation clip: " << filepath << "\n";
            return false;
        }

        // Hydrate the actual runtime object
        auto new_clip = std::make_shared<AnimationClip>(loaded_data.name, loaded_data.width, loaded_data.height);
        new_clip->SetFPS(loaded_data.fps);

        s_active_clip = new_clip;
        s_active_clip_path = filepath;
        return true;
    }

    bool AnimationManager::SaveActiveClip() {
        if (!s_active_clip || s_active_clip_path.empty()) return false;

        ClipSaveData data_to_save{
            s_active_clip->GetName(),
            s_active_clip->GetWidth(),
            s_active_clip->GetHeight(),
            s_active_clip->GetFPS()
        };

        std::string json_buffer;
        glz::write_json(data_to_save, json_buffer);

        std::ofstream file(s_active_clip_path);
        if (!file.is_open()) return false;
        file << json_buffer;
        return true;
    }
}