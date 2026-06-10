#include "ImageManager.h"
#include "ProjectManager.h" // Updated component state binding source routing
#include "picsel/utility/PngWriter.h"      // Binary chunk assembly toolkit encoder hook
#include <algorithm>
#include <sstream>

#include "asset/assetloading/BatchLoader.h" // Extension hook for bulk asset import operations

#include <iostream>

namespace picsel {

    ImageManager* ImageManager::s_instance = nullptr;

    ImageManager::ImageManager() { 
        s_instance = this;
    }
    ImageManager::~ImageManager() { if (s_instance == this) s_instance = nullptr; }

    bool ImageManager::IsProjectActive() const {
        return ProjectManager::Get() && ProjectManager::Get()->HasActiveProject();
    }

    VirtualFolder* ImageManager::GetRootFolder() {
        if (!IsProjectActive()) return nullptr;
        return &ProjectManager::Get()->GetActiveProject().value().virtual_root;
    }

    std::vector<std::string> ImageManager::SplitPath(const std::string& path_str) const {
        std::vector<std::string> elements;
        std::stringstream ss(path_str);
        std::string item;
        while (std::getline(ss, item, '/')) {
            if (!item.empty()) elements.push_back(item);
        }
        return elements;
    }

    VirtualFolder* ImageManager::ResolvePathString(const std::string& path_str) {
        VirtualFolder* root = GetRootFolder();
        if (!root) return nullptr;

        auto elements = SplitPath(path_str);
        if (elements.empty() || elements[0] != root->name) return nullptr;

        VirtualFolder* current = root;
        for (size_t i = 1; i < elements.size(); ++i) {
            auto it = std::find_if(current->subfolders.begin(), current->subfolders.end(),
                [&](const VirtualFolder& f) { return f.name == elements[i]; });

            if (it != current->subfolders.end()) {
                current = &(*it);
            }
            else {
                return nullptr;
            }
        }
        return current;
    }

    void ImageManager::SetCurrentPathString(const std::string& path_str) {
        if (IsProjectActive()) {
            m_current_path_str = path_str;
            VerifyCurrentPath();
        }
    }

    void ImageManager::VerifyCurrentPath() {
        if (!ResolvePathString(m_current_path_str)) {
            m_current_path_str = "Root";
        }
    }

    bool ImageManager::IsCanvasNameGloballyTaken(const std::string& name) const {
        if (!IsProjectActive()) return false;
        return SearchNameConflictRecursive(ProjectManager::Get()->GetActiveProject().value().virtual_root, name);
    }

    bool ImageManager::SearchNameConflictRecursive(const VirtualFolder& current, const std::string& name) const {
        for (const auto& file : current.files) {
            if (file.name == name) return true;
        }
        for (const auto& sub : current.subfolders) {
            if (SearchNameConflictRecursive(sub, name)) return true;
        }
        return false;
    }

    void ImageManager::CreateVirtualFolder(const std::string& parent_path_str, const std::string& base_name) {
        VirtualFolder* parent = ResolvePathString(parent_path_str);
        if (!parent) return;

        std::string final_name = base_name;
        int counter = 1;
        while (std::any_of(parent->subfolders.begin(), parent->subfolders.end(),
            [&](const VirtualFolder& f) { return f.name == final_name; })) {
            final_name = base_name + " (" + std::to_string(counter++) + ")";
        }

        VirtualFolder new_folder;
        new_folder.name = final_name;
        parent->subfolders.push_back(std::move(new_folder));
    }
    

    void ImageManager::RenameCanvasFile(const std::string& parent_path_str, int file_idx, const std::string& new_name) {
        VirtualFolder* folder = ResolvePathString(parent_path_str);
        if (!folder || file_idx < 0 || file_idx >= static_cast<int>(folder->files.size())) return;

        if (folder->files[file_idx].name == new_name) return;

        if (IsCanvasNameGloballyTaken(new_name)) {
            throw std::runtime_error("Canvas conflict: '" + new_name + "' already exists in this project.");
        }

        folder->files[file_idx].name = new_name;
    }

    bool ImageManager::MoveFolderVirtual(const std::string& source_path, const std::string& dest_path) {
        if (source_path == dest_path) return false;
        if (dest_path.rfind(source_path + "/", 0) == 0) return false;

        size_t last_slash = source_path.find_last_of('/');
        if (last_slash == std::string::npos) return false;
        std::string src_parent_path = source_path.substr(0, last_slash);
        std::string src_folder_name = source_path.substr(last_slash + 1);

        VirtualFolder* src_parent = ResolvePathString(src_parent_path);
        VirtualFolder* dest_folder = ResolvePathString(dest_path);
        if (!src_parent || !dest_folder) return false;

        auto it = std::find_if(src_parent->subfolders.begin(), src_parent->subfolders.end(),
            [&](const VirtualFolder& f) { return f.name == src_folder_name; });

        if (it != src_parent->subfolders.end()) {
            std::string final_name = it->name;
            int counter = 1;
            while (std::any_of(dest_folder->subfolders.begin(), dest_folder->subfolders.end(),
                [&](const VirtualFolder& f) { return f.name == final_name; })) {
                final_name = it->name + " (" + std::to_string(counter++) + ")";
            }

            VirtualFolder moved = std::move(*it);
            moved.name = final_name;
            src_parent->subfolders.erase(it);
            dest_folder->subfolders.push_back(std::move(moved));

            VerifyCurrentPath();
            return true;
        }
        return false;
    }

    bool ImageManager::MoveFileVirtual(const std::string& source_parent_path, int file_idx, const std::string& dest_path) {
        VirtualFolder* src_parent = ResolvePathString(source_parent_path);
        VirtualFolder* dest_folder = ResolvePathString(dest_path);
        if (!src_parent || !dest_folder || source_parent_path == dest_path) return false;
        if (file_idx < 0 || file_idx >= static_cast<int>(src_parent->files.size())) return false;

        VirtualFile target_file = src_parent->files[file_idx];
        dest_folder->files.push_back(std::move(target_file));
        src_parent->files.erase(src_parent->files.begin() + file_idx);
        return true;
    }

    void ImageManager::SyncWithGPU() {
        if (!IsProjectActive()) return;

        // Grab the physical frames directory where all actual disk PNGs live
        auto* proj_mgr = ProjectManager::Get();
        std::filesystem::path frames_dir = proj_mgr->GetActiveProjectPath().parent_path() / "frames";

        // Dispatch command to your engine's asset ingestion system
        // (Assuming ReloadDirectory is the static method matching your snippet)
        app::BatchLoader::ReloadDirectory(frames_dir);
    }

    void ImageManager::CreateCanvasFile(const std::string& target_path_str, const std::string& requested_name) {
        VirtualFolder* folder = ResolvePathString(target_path_str);
        if (!folder) return;

        std::string final_name = requested_name;
        int counter = 1;
        while (IsCanvasNameGloballyTaken(final_name)) {
            final_name = requested_name + "_Conflict(" + std::to_string(counter++) + ")";
        }

        VirtualFile new_file;
        new_file.name = final_name;

        // Either use ".png" or your specific engine extension like ".img.gbe" 
        // if your BatchLoader enforces it strictly.
        auto basename = "canvas_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

        new_file.backend_id = basename;
        new_file.actual_png_name = basename + ".png";

        auto* proj_mgr = ProjectManager::Get();
        std::filesystem::path absolute_png_path = proj_mgr->GetActiveProjectPath().parent_path() / "frames" / new_file.actual_png_name;

        if (PngWriter::WriteEmptyRGBA(absolute_png_path, DEFAULT_IMAGE_DIMS, DEFAULT_IMAGE_DIMS)) {
            folder->files.push_back(std::move(new_file));

            // --> TRIGGER GPU RELOAD AFTER SUCCESSFUL CREATION
            SyncWithGPU();
        }
        else {
            std::cerr << "[Picsel] Failed to create physical texture track asset on disk.\n";
        }
    }

    void ImageManager::DeleteCanvasFile(const std::string& parent_path_str, int file_idx) {
        VirtualFolder* folder = ResolvePathString(parent_path_str);
        if (!folder || file_idx < 0 || file_idx >= static_cast<int>(folder->files.size())) return;

        std::string target_physical_name = folder->files[file_idx].actual_png_name;

        // Remove virtual node
        folder->files.erase(folder->files.begin() + file_idx);

        // Remove physical disk asset
        auto* proj_mgr = ProjectManager::Get();
        std::filesystem::path physical_path = proj_mgr->GetActiveProjectPath().parent_path() / "frames" / target_physical_name;

        if (std::filesystem::exists(physical_path)) {
            std::filesystem::remove(physical_path);
        }

        // --> TRIGGER GPU RELOAD AFTER DELETION
        SyncWithGPU();
    }

    void ImageManager::DeleteVirtualFolder(const std::string& target_path_str) {
        if (target_path_str == "Root") return; // Protect root

        size_t last_slash = target_path_str.find_last_of('/');
        if (last_slash == std::string::npos) return;

        std::string parent_path = target_path_str.substr(0, last_slash);
        std::string folder_name = target_path_str.substr(last_slash + 1);

        VirtualFolder* parent = ResolvePathString(parent_path);
        if (!parent) return;

        auto it = std::find_if(parent->subfolders.begin(), parent->subfolders.end(),
            [&](const VirtualFolder& f) { return f.name == folder_name; });

        if (it != parent->subfolders.end()) {
            // Because our virtual folder system abstracts the disk, physical 
            // files inside this virtual group must be queried recursively to delete 
            // them off the disk if you wish to destroy all associated PNGs as well.
            // (For now, just clearing the virtual branch. Implement a recursive physical deletion pass if desired).

            parent->subfolders.erase(it);
            VerifyCurrentPath();

            // --> TRIGGER GPU RELOAD JUST IN CASE PHYSICAL FILES WERE REMOVED
            SyncWithGPU();
        }
    }
}