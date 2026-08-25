#pragma once

#include "gui/features/menubar/MenuBarExtension.h"
#include "app/ProjectLoader.hpp"
#include "app/App.hpp"
#include "FileDialogue.hpp"
#include "CreateInstance.hpp"
#include <imgui.h>

namespace gsr {

class MenuBarExtension : public app::MenuBarExtension {
public:
    MenuBarExtension() = default;
    ~MenuBarExtension() = default;

    void DrawMenuBarMenu() override {
        // --- File Menu ---
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project", "Ctrl+N")) {
                App::GetInstance().SaveUndoPoint();
                ProjectLoader::StartNewProject();
            }

            if (ImGui::MenuItem("Load Project", "Ctrl+O")) {
                std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::OPEN, "gsrproj");
                if (!outPath.empty()) {
                    ProjectLoader::LoadProject(outPath);
                }
            }

            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                if (!ProjectLoader::QuickSave()) {
                    std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "gsrproj");
                    if (!outPath.empty()) {
                        ProjectLoader::SaveProject(outPath);
                    }
                }
            }

            if (ImGui::MenuItem("Save Project As...")) {
                std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "gsrproj");
                if (!outPath.empty()) {
                    ProjectLoader::SaveProject(outPath);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                App::GetInstance().shutdown();
            }

            ImGui::EndMenu();
        }

        // --- Edit Menu ---
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                App::GetInstance().Undo();
            }

            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                App::GetInstance().Redo();
            }

            ImGui::EndMenu();
        }
    }
};

} // namespace gsr