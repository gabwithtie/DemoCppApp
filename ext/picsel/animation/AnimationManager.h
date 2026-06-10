#pragma once
#include "AnimationClip.h"
#include <memory>
#include <filesystem>
#include <string>

namespace picsel {
    class AnimationManager {
    public:
        inline AnimationManager() {}
        inline ~AnimationManager() { SaveActiveClip(); }

        // --- Active State ---
        static std::shared_ptr<AnimationClip> GetActiveClip();
        static void SetActiveClip(std::shared_ptr<AnimationClip> clip);
        static std::string GetActiveClipPath();

        // --- File Operations ---
        static bool CreateNewClip(const std::filesystem::path& directory, const std::string& name, int width, int height);
        static bool LoadClip(const std::filesystem::path& filepath);
        static bool SaveActiveClip();

    private:
        inline static std::shared_ptr<AnimationClip> s_active_clip = nullptr;
        inline static std::filesystem::path s_active_clip_path = "";
    };
}