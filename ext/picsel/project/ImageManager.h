#pragma once

#include "ProjectData.h"
#include <string>
#include <vector>
#include <stdexcept>

namespace picsel {

    class ImageManager {
    public:
        ImageManager();
        ~ImageManager();

        static ImageManager* Get() { return s_instance; }

        bool IsProjectActive() const;

        // Directory Tree Resolution
        VirtualFolder* GetRootFolder();
        VirtualFolder* ResolvePathString(const std::string& path_str);

        std::string GetCurrentPathString() const { return m_current_path_str; }
        void SetCurrentPathString(const std::string& path_str);

        // Naming Rules Enforcement
        bool IsCanvasNameGloballyTaken(const std::string& name) const;

        // Virtual Directory Manipulations
        void CreateVirtualFolder(const std::string& parent_path_str, const std::string& base_name);
        void CreateCanvasFile(const std::string& target_path_str, const std::string& requested_name);
        void RenameCanvasFile(const std::string& parent_path_str, int file_idx, const std::string& new_name);

        // --- NEW: Deletion Handlers ---
        void DeleteCanvasFile(const std::string& parent_path_str, int file_idx);
        void DeleteVirtualFolder(const std::string& target_path_str);

        bool MoveFolderVirtual(const std::string& source_path_str, const std::string& dest_path_str);
        bool MoveFileVirtual(const std::string& source_parent_path_str, int file_idx, const std::string& dest_path_str);

        // --- NEW: Hardware Synchronization ---
        void SyncWithGPU();

    private:
        std::vector<std::string> SplitPath(const std::string& path_str) const;
        bool SearchNameConflictRecursive(const VirtualFolder& current, const std::string& name) const;
        void VerifyCurrentPath();

        static ImageManager* s_instance;
        std::string m_current_path_str = "Root";
    };
}