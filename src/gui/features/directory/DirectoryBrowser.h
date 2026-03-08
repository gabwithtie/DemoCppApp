#pragma once

#include "gui/main/GuiWindow.h"

#include <filesystem>
#include <vector>
#include <functional>
#include <map>

namespace app {

    using FileOpenerFunc = std::function<void(const std::filesystem::path&)>;

    class DirectoryBrowser : public GuiWindow {
    public:
        DirectoryBrowser();
        ~DirectoryBrowser();

        // Singleton Access
        static DirectoryBrowser* Get() { return s_instance; }
        static void SetProjectDirectory(std::filesystem::path new_path);

        // File Opener Registry
        static void RegisterOpener(const std::string& extension, FileOpenerFunc func);

        std::string GetWindowId() override { return "Project Browser"; }

        inline void SetOnSelectCallback(std::function<void(std::filesystem::path)> callback) {
            on_select_callback = callback;
        }

    protected:
        void DrawSelf() override;

    private:
        std::function<void(std::filesystem::path)> on_select_callback = nullptr;

        static DirectoryBrowser* s_instance;
        static std::map<std::string, FileOpenerFunc> s_openers;

        std::filesystem::path root_path;
        std::filesystem::path current_path;

        // Clipboard State
        std::filesystem::path clipboard_path;
        bool is_cut_operation = false;

        float thumbnail_size = 64.0f;
        char search_filter[128] = "";

        void DrawTree(const std::filesystem::path& path);
        void DrawContentArea();
        void DrawBreadcrumbs();

        // File Actions
        void CreateFolder(const std::filesystem::path& at_path);
        void DeletePath(const std::filesystem::path& path);
        void Paste(const std::filesystem::path& destination);
    };
}