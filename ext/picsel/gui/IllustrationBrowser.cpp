#include "IllustrationBrowser.h"
#include "picsel/illustration/IllustrationManager.h"
#include <imgui.h>

namespace picsel {

    IllustrationBrowser::IllustrationBrowser() {
        // Set up any initial structural constraints if needed, e.g., flags.
    }

    void IllustrationBrowser::DrawSelf() {
        // ImGui dual-column layout: Left sidebar for lists, Right for active properties

        ImGui::Columns(2, "IllustrationManagerSplitter", true);

        // --- COLUMN 1: Sidebar List ---
        DrawSidebarList();
        ImGui::NextColumn();

        // --- COLUMN 2: Workspace Details ---
        DrawActiveIllustrationWorkspace();
        ImGui::Columns(1); // Reset layout column state

        // --- DEFERRED INTERFACE OVERLAYS ---
        DrawCreateIllustrationModal();
    }

    void IllustrationBrowser::DrawSidebarList() {
        ImGui::TextDisabled("PROJECT COMPOSITIONS");
        ImGui::Separator();

        // Button to trigger the creation overlay logic
        if (ImGui::Button("+ New Illustration", ImVec2(-1, 0))) {
            m_open_create_modal = true;
        }
        ImGui::Spacing();

        const auto& illustrations = IllustrationManager::getAllIllustrations();
        IllustrationData* activeIllus = IllustrationManager::getActiveIllustration();

        ImGui::BeginChild("IllustrationSelectableList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
        for (size_t i = 0; i < illustrations.size(); ++i) {
            // Your custom implementation accepts both index or std::string name. 
            // We use the file stem (name without path/extension) to display nicely.
            std::string displayName = illustrations[i].name;

            bool isSelected = (activeIllus != nullptr && activeIllus->name == illustrations[i].name);

            if (ImGui::Selectable(displayName.c_str(), isSelected)) {
                IllustrationManager::setActiveIllustration(i);
            }

            // Provide visual feedback if this element is currently acting as active context
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndChild();

        // Universal Explicit Quick Save Button at bottom of sidebar
        if (ImGui::Button("Save Current", ImVec2(-1, 0))) {
            IllustrationManager::saveActiveIllustration();
        }
    }

    void IllustrationBrowser::DrawActiveIllustrationWorkspace() {
        IllustrationData* activeIllus = IllustrationManager::getActiveIllustration();

        if (!activeIllus) {
            ImGui::BeginChild("EmptyWorkspacePlaceholder");
            ImGui::TextDisabled("No active illustration loaded.");
            ImGui::TextDisabled("Select or create a file composition from the left sidebar.");
            ImGui::EndChild();
            return;
        }

        // Header containing properties
        ImGui::Text("Active Composition: %s", activeIllus->name.c_str());
        ImGui::Separator();
        ImGui::Spacing();

        // Layer Manifest Inspection panel
        ImGui::Text("Layer Manifest (%d total)", (int)activeIllus->layerFilenames.size());

        ImGui::BeginChild("LayersSubList", ImVec2(0, 200), true);
        for (size_t i = 0; i < activeIllus->layerFilenames.size(); ++i) {
            ImGui::BulletText("%s", activeIllus->layerFilenames[i].c_str());
        }
        if (activeIllus->layerFilenames.empty()) {
            ImGui::TextDisabled("  Empty Composition (No layers registered)");
        }
        ImGui::EndChild();

        // Transient Layer Control operations mockup
        static char buf_layer_name[128] = "";
        ImGui::InputText("##NewLayerBuffer", buf_layer_name, IM_ARRAYSIZE(buf_layer_name));
        ImGui::SameLine();
        if (ImGui::Button("Add Layer Asset")) {
            if (strlen(buf_layer_name) > 0) {
                IllustrationManager::createNewLayerForActive(buf_layer_name);
                buf_layer_name[0] = '\0'; // Clear transient buffer
            }
        }
    }

    void IllustrationBrowser::DrawCreateIllustrationModal() {
        // Handle deferred popup lifecycle hook securely without breaking column stacks
        if (m_open_create_modal) {
            ImGui::OpenPopup("Create New Composition");
            m_open_create_modal = false;
        }

        if (ImGui::BeginPopupModal("Create New Composition", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Specify file structure configuration context below:");
            ImGui::Spacing();

            ImGui::InputText("Asset Identity File Name", m_buf_new_name, IM_ARRAYSIZE(m_buf_new_name));
            ImGui::TextDisabled("System targets target workspace folder automatically with standard .illus suffix.");

            ImGui::Separator();

            if (ImGui::Button("Generate Asset", ImVec2(120, 0))) {
                if (strlen(m_buf_new_name) > 0) {
                    IllustrationManager::createNewIllustration(m_buf_new_name);
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

} // namespace picsel