#include "ImageBrowser.h"
#include "picsel/project/ImageManager.h"
#include "picsel/viewport/Viewport.h"
#include "imgui.h"
#include <sstream>

namespace picsel {

    static char s_rename_buf[128] = "";
    static int s_editing_file_idx = -1;
    static std::string s_error_notification = "";

    void ImageBrowser::DrawSelf() {
        ImageManager* manager = ImageManager::Get();
        if (!manager || !manager->IsProjectActive()) {
            ImGui::Text("Please open or load a valid project to inspect assets.");
            return;
        }

        DrawTopActionBar();
        ImGui::Separator();
        DrawBreadcrumbs();
        ImGui::Separator();

        if (!s_error_notification.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", s_error_notification.c_str());
            ImGui::SameLine();
            if (ImGui::Button("Clear")) s_error_notification.clear();
            ImGui::Separator();
        }

        ImGui::BeginChild("VirtualTreeSide", ImVec2(220.0f, 0), true);
        DrawTreeSide(manager->GetRootFolder(), "Root");
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("VirtualContentArea", ImVec2(0, 0), false);
        DrawContentArea();
        ImGui::EndChild();
    }

    void ImageBrowser::DrawTopActionBar() {
        ImageManager* manager = ImageManager::Get();
        std::string current_path = manager->GetCurrentPathString();

        if (ImGui::Button("+ New Canvas")) {
            s_error_notification.clear();
            manager->CreateCanvasFile(current_path, "Untitled_Canvas");
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Import PNG")) { /* Extension Hook Later */ }
        ImGui::SameLine();
        if (ImGui::Button("+ New Virtual Group")) {
            manager->CreateVirtualFolder(current_path, "New Group");
        }
    }

    void ImageBrowser::DrawBreadcrumbs() {
        ImageManager* manager = ImageManager::Get();
        std::string path_str = manager->GetCurrentPathString();

        std::stringstream ss(path_str);
        std::string token;
        std::string accumulated_path = "";
        bool first = true;

        while (std::getline(ss, token, '/')) {
            if (token.empty()) continue;
            if (!first) { ImGui::SameLine(); ImGui::Text(">"); ImGui::SameLine(); }

            accumulated_path += (first ? "" : "/") + token;
            first = false;

            if (ImGui::Selectable(token.c_str(), path_str == accumulated_path, 0, ImVec2(0, 0))) {
                manager->SetCurrentPathString(accumulated_path);
            }
        }
    }

    void ImageBrowser::DrawTreeSide(VirtualFolder* folder, const std::string& current_node_path) {
        if (!folder) return;
        ImageManager* manager = ImageManager::Get();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (manager->GetCurrentPathString() == current_node_path) flags |= ImGuiTreeNodeFlags_Selected;
        if (folder->subfolders.empty()) flags |= ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx((void*)folder, flags, "%s", folder->name.c_str());

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            manager->SetCurrentPathString(current_node_path);
        }

        // --- ImGui Drop Targets using stable Path-Strings ---
        if (current_node_path != "Root" && ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("VIRTUAL_FOLDER_PATH", current_node_path.c_str(), current_node_path.size() + 1);
            ImGui::Text("Moving folder: %s", folder->name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VIRTUAL_FOLDER_PATH")) {
                std::string source_path((const char*)payload->Data);
                manager->MoveFolderVirtual(source_path, current_node_path);
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VIRTUAL_FILE_INDEX")) {
                std::string payload_str((const char*)payload->Data);
                size_t split_pos = payload_str.find_last_of('|');
                std::string src_parent = payload_str.substr(0, split_pos);
                int file_idx = std::stoi(payload_str.substr(split_pos + 1));

                manager->MoveFileVirtual(src_parent, file_idx, current_node_path);
            }
            ImGui::EndDragDropTarget();
        }

        if (open) {
            for (auto& sub : folder->subfolders) {
                DrawTreeSide(&sub, current_node_path + "/" + sub.name);
            }
            ImGui::TreePop();
        }
    }

    void ImageBrowser::DrawContentArea() {
        ImageManager* manager = ImageManager::Get();
        std::string current_path = manager->GetCurrentPathString();
        VirtualFolder* current_dir = manager->ResolvePathString(current_path);

        if (!current_dir) return;

        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        float padding = 16.0f;
        float cell_size = m_thumbnail_size + padding;

        // Render Folder Icons
        for (auto& sub : current_dir->subfolders) {
            std::string sub_path = current_path + "/" + sub.name;
            ImGui::PushID(sub_path.c_str());
            ImGui::BeginGroup();

            ImGui::Button("[ Folder ]", ImVec2(m_thumbnail_size, m_thumbnail_size));

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                manager->SetCurrentPathString(sub_path);
            }

            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("VIRTUAL_FOLDER_PATH", sub_path.c_str(), sub_path.size() + 1);
                ImGui::Text("Moving %s", sub.name.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("VIRTUAL_FOLDER_PATH")) {
                    std::string source_path((const char*)payload->Data);
                    manager->MoveFolderVirtual(source_path, sub_path);
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::TextWrapped("%s", sub.name.c_str());
            ImGui::EndGroup();

            float last_button_x2 = ImGui::GetItemRectMax().x;
            if (last_button_x2 + padding + cell_size < window_visible_x2) ImGui::SameLine();
            ImGui::PopID();
        }

        // Render Canvas Icons
        for (int i = 0; i < static_cast<int>(current_dir->files.size()); ++i) {
            auto& file = current_dir->files[i];
            ImGui::PushID(i);
            ImGui::BeginGroup();

            ImGui::Button("[ PNG ]", ImVec2(m_thumbnail_size, m_thumbnail_size));

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (picsel::Viewport::Get()) {
                    picsel::Viewport::Get()->SetActiveCanvas(file.actual_png_name);
                }
            }

            if (ImGui::BeginDragDropSource()) {
                std::string file_payload = current_path + "|" + std::to_string(i);
                ImGui::SetDragDropPayload("VIRTUAL_FILE_INDEX", file_payload.c_str(), file_payload.size() + 1);
                ImGui::Text("Moving item: %s", file.name.c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginPopupContextItem("CanvasContextMenu")) { // Native ImGui Right-Click
                if (ImGui::MenuItem("Rename")) {
                    s_editing_file_idx = i;
                    strcpy(s_rename_buf, file.name.c_str());
                }
                if (ImGui::MenuItem("Delete File")) {
                    manager->DeleteCanvasFile(current_path, i);
                }
                ImGui::EndPopup();
            }

            if (s_editing_file_idx == i) {
                ImGui::SetNextItemWidth(m_thumbnail_size);
                if (ImGui::InputText("##Rename", s_rename_buf, IM_ARRAYSIZE(s_rename_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    try {
                        s_error_notification.clear();
                        manager->RenameCanvasFile(current_path, i, s_rename_buf);
                        s_editing_file_idx = -1;
                    }
                    catch (const std::runtime_error& err) {
                        s_error_notification = err.what();
                    }
                }
                if (ImGui::IsItemDeactivated() && !ImGui::IsKeyPressed(ImGuiKey_Enter)) s_editing_file_idx = -1;
            }
            else {
                ImGui::TextWrapped("%s", file.name.c_str());
                if (ImGui::IsItemClicked()) {
                    s_editing_file_idx = i;
                    strcpy(s_rename_buf, file.name.c_str());
                }
            }

            ImGui::EndGroup();

            float last_button_x2 = ImGui::GetItemRectMax().x;
            if (i < static_cast<int>(current_dir->files.size()) - 1 && last_button_x2 + padding + cell_size < window_visible_x2) {
                ImGui::SameLine();
            }
            ImGui::PopID();
        }
    }
}