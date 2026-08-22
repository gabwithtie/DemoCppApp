#include "DirectoryBrowser.h"

#include <iostream>

#include <imgui.h>

namespace app {

    DirectoryBrowser* DirectoryBrowser::s_instance = nullptr;
    std::map<std::string, FileOpenerFunc> DirectoryBrowser::s_openers;

    DirectoryBrowser::DirectoryBrowser() {
        s_instance = this;
        root_path = std::filesystem::current_path();
        current_path = root_path;
    }

    DirectoryBrowser::~DirectoryBrowser() {
        if (s_instance == this) s_instance = nullptr;
    }

    void DirectoryBrowser::SetProjectDirectory(std::filesystem::path new_path) {
        if (s_instance && std::filesystem::exists(new_path)) {
            s_instance->root_path = std::filesystem::absolute(new_path);
            s_instance->current_path = s_instance->root_path;
        }
    }

    void DirectoryBrowser::RegisterOpener(const std::string& extension, FileOpenerFunc func) {
        s_openers[extension] = func;
    }

    void DirectoryBrowser::DrawSelf() {
        DrawBreadcrumbs();
        ImGui::Separator();

        static float left_pane_width = 200.0f;
        ImGui::BeginChild("TreeSide", ImVec2(left_pane_width, 0), true);
        DrawTree(root_path);
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::InvisibleButton("v_splitter", ImVec2(5.0f, -1.0f));
        if (ImGui::IsItemActive()) left_pane_width += ImGui::GetIO().MouseDelta.x;
        ImGui::SameLine();

        ImGui::BeginChild("ContentSide", ImVec2(0, 0), true);

        if (ImGui::BeginPopupContextWindow("ContentContext", ImGuiMouseButton_Right)) {
            if (ImGui::MenuItem("New Folder")) CreateFolder(current_path);
            if (ImGui::MenuItem("Paste", "Ctrl+V", false, !clipboard_path.empty())) Paste(current_path);
            ImGui::EndPopup();
        }

        DrawContentArea();
        ImGui::EndChild();
    }

    void DirectoryBrowser::DrawContentArea() {
        float padding = 16.0f;
        float cellSize = thumbnail_size + padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1) columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        static std::filesystem::path renaming_path;
        static char rename_buffer[256];
        static bool set_focus_next_frame = false;

        try {
            for (auto& entry : std::filesystem::directory_iterator(current_path)) {
                auto& metapath = entry.path();
                std::filesystem::path actualpath = metapath;
                bool is_dir = entry.is_directory();
                std::filesystem::path parentpath = metapath.parent_path();

                void* texdata = nullptr;

                if (is_dir) {
                    //assign icon here
                }

                std::string filename = actualpath.filename().string();
                if (strlen(search_filter) > 0 && filename.find(search_filter) == std::string::npos)
                    continue;
                ImGui::PushID(filename.c_str());

                // --- 1. Icon Button ---
                const auto ui_func = [&] {
                    auto size = ImVec2(thumbnail_size, thumbnail_size);
                    if (texdata == nullptr)
                        ImGui::Button("##item", size);
                    else
                        ImGui::ImageButton("##item", 0, size, ImVec2(0, 0), ImVec2(1, 1));
                    };

                if (is_dir)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.45f, 0.7f, 1.0f));
                    ui_func();
                }
                else
                {
                    ui_func();
                }

                // Single Click / Select Callback
                if (ImGui::IsItemClicked(0) && on_select_callback) {
                    on_select_callback(actualpath);
                }

                // Double Click to Open
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    if (is_dir) current_path = actualpath;
                    else {
                        std::string ext = actualpath.extension().string();
                        if (s_openers.count(ext)) s_openers[ext](actualpath);
                    }
                }
                if (is_dir) ImGui::PopStyleColor();

                // Context Menu
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Rename")) {
                        renaming_path = actualpath;
                        strncpy(rename_buffer, filename.c_str(), 256);
                        set_focus_next_frame = true;
                    }
                    if (ImGui::MenuItem("Copy", "Ctrl+C")) { clipboard_path = actualpath; is_cut_operation = false; }
                    if (ImGui::MenuItem("Cut", "Ctrl+X")) { clipboard_path = actualpath; is_cut_operation = true; }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Delete", "Del")) DeletePath(actualpath);
                    ImGui::EndPopup();
                }

                // --- 2. Label / Rename Logic ---
                if (renaming_path == actualpath) {
                    if (set_focus_next_frame) { ImGui::SetKeyboardFocusHere(); set_focus_next_frame = false; }
                    ImGui::SetNextItemWidth(thumbnail_size);

                    bool is_empty = (strlen(rename_buffer) == 0);
                    if (is_empty) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));

                    if (ImGui::InputText("##renamebox", rename_buffer, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                        if (!is_empty) {
                            std::filesystem::rename(actualpath, actualpath.parent_path() / rename_buffer);
                            renaming_path.clear();
                        }
                    }
                    if (is_empty) ImGui::PopStyleColor();
                    if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) renaming_path.clear();
                }
                else {
                    ImGui::TextWrapped(filename.c_str());

                    // Select on Label Click
                    if (ImGui::IsItemClicked(0) && on_select_callback) {
                        on_select_callback(actualpath);
                    }

                    // Rename on Label Double Click
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        renaming_path = actualpath;
                        strncpy(rename_buffer, filename.c_str(), 256);
                        set_focus_next_frame = true;
                    }
                }

                ImGui::NextColumn();
                ImGui::PopID();
            }
        }
        catch (...) {}
        ImGui::Columns(1);
    }

    void DirectoryBrowser::DrawBreadcrumbs() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 4));
        std::vector<std::filesystem::path> parts;
        for (auto p = current_path; p != root_path.parent_path() && !p.empty(); p = p.parent_path())
            parts.push_back(p);

        for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
            if (ImGui::Button(it->filename().string().c_str())) {
                current_path = *it;
                if (on_select_callback) on_select_callback(*it);
            }
            if (std::next(it) != parts.rend()) {
                ImGui::SameLine(); ImGui::TextDisabled(">"); ImGui::SameLine();
            }
        }

        // Fixed width search bar at the right
        float search_width = 150.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - (search_width + 25.0f));
        ImGui::SetNextItemWidth(search_width);
        ImGui::InputTextWithHint("##Search", "Search...", search_filter, 128);

        ImGui::PopStyleVar();
    }

    void DirectoryBrowser::DrawTree(const std::filesystem::path& path) {
        ImGuiTreeNodeFlags flags = ((current_path == path) ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        bool has_subdirs = false;
        try {
            for (auto const& entry : std::filesystem::directory_iterator(path))
                if (entry.is_directory()) { has_subdirs = true; break; }
        }
        catch (...) {}
        if (!has_subdirs) flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx(path.filename().string().c_str(), flags);

        if (ImGui::IsItemClicked()) {
            current_path = path;
            if (on_select_callback) on_select_callback(path);
        }

        if (open) {
            try {
                for (auto const& entry : std::filesystem::directory_iterator(path))
                    if (entry.is_directory()) DrawTree(entry.path());
            }
            catch (...) {}
            ImGui::TreePop();
        }
    }

    void DirectoryBrowser::CreateFolder(const std::filesystem::path& at_path) {
        std::filesystem::path p = at_path / "New Folder";
        int i = 1;
        while (std::filesystem::exists(p)) p = at_path / ("New Folder (" + std::to_string(i++) + ")");
        std::filesystem::create_directory(p);
    }

    void DirectoryBrowser::DeletePath(const std::filesystem::path& path) {
        try {
            std::filesystem::remove_all(path);
        }
        catch (...) {}
    }

    void DirectoryBrowser::Paste(const std::filesystem::path& destination) {
        if (clipboard_path.empty()) return;
        try {
            auto target = destination / clipboard_path.filename();
            if (is_cut_operation) {
                std::filesystem::rename(clipboard_path, target);
                clipboard_path.clear();
            }
            else {
                std::filesystem::copy(clipboard_path, target, std::filesystem::copy_options::recursive);
            }
        }
        catch (...) {}
    }
}