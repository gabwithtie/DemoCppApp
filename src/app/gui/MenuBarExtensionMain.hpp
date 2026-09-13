#pragma once

#include "gui/features/menubar/MenuBarExtension.h"
#include "app/ProjectLoader.hpp"
#include "app/ProjectExporter.hpp"
#include "app/App.hpp"
#include "FileDialogue.hpp"
#include "CreateInstance.hpp"
#include <imgui.h>

#ifndef APP_MAX_CANVAS_PIXEL_SIZE
#define APP_MAX_CANVAS_PIXEL_SIZE 4096
#endif

namespace app {

class MenuBarExtensionMain : public app::MenuBarExtension {
public:
    MenuBarExtensionMain() = default;
    ~MenuBarExtensionMain() = default;

    void DrawMenuBarMenu() override {
        // --- File Menu ---
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project", "Ctrl+N")) {
                ProjectLoader::StartNewProject();
            }

            if (ImGui::MenuItem("Load Project", "Ctrl+O")) {
                std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::OPEN, "appproj");
                if (!outPath.empty()) {
                    ProjectLoader::LoadProject(outPath);
                }
            }

            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                if (!ProjectLoader::QuickSave()) {
                    std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "appproj");
                    if (!outPath.empty()) {
                        ProjectLoader::SaveProject(outPath);
                    }
                }
            }

            if (ImGui::MenuItem("Save Project As...")) {
                std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "appproj");
                if (!outPath.empty()) {
                    ProjectLoader::SaveProject(outPath);
                }
            }

            if (ImGui::MenuItem("Export to PNG")) {
                std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "png");
                if (!outPath.empty()) {
                    ProjectExporter::ExportProjectToPng(App::GetInstance().project, outPath);
                }
            }

            if (ImGui::MenuItem("Canvas Size...")) {
                App& app = App::GetInstance();
                m_canvasWidthDraft = ClampCanvasDimension(app.project.w);
                m_canvasHeightDraft = ClampCanvasDimension(app.project.h);
                m_shouldOpenCanvasSizePopup = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                App::GetInstance().shutdown();
            }

            ImGui::EndMenu();
        }

        DrawCanvasSizePopup();
    }

private:
    static int ClampCanvasDimension(int value) {
        if (value < 1) {
            return 1;
        }
        if (value > APP_MAX_CANVAS_PIXEL_SIZE) {
            return APP_MAX_CANVAS_PIXEL_SIZE;
        }
        return value;
    }

    void ApplyCanvasResize(int width, int height) {
        const int clampedWidth = ClampCanvasDimension(width);
        const int clampedHeight = ClampCanvasDimension(height);

        App& app = App::GetInstance();
        if (app.project.w == clampedWidth && app.project.h == clampedHeight) {
            return;
        }

        app.edit_history.Execute(app.project, "Resize Canvas", [clampedWidth, clampedHeight](Model::Project& project) {
            project.w = clampedWidth;
            project.h = clampedHeight;
            for (auto& layer : project.layers) {
                layer.EnsureSize(clampedWidth, clampedHeight, Model::Layer::Transparent);
            }
        });
    }

    void DrawCanvasSizePopup() {
        if (m_shouldOpenCanvasSizePopup) {
            ImGui::OpenPopup("Canvas Size");
            m_shouldOpenCanvasSizePopup = false;
        }

        if (!ImGui::BeginPopupModal("Canvas Size", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }

        ImGui::Text("Set canvas pixel size (max %d)", APP_MAX_CANVAS_PIXEL_SIZE);
        ImGui::InputInt("Width", &m_canvasWidthDraft);
        ImGui::InputInt("Height", &m_canvasHeightDraft);

        m_canvasWidthDraft = ClampCanvasDimension(m_canvasWidthDraft);
        m_canvasHeightDraft = ClampCanvasDimension(m_canvasHeightDraft);

        if (ImGui::Button("Apply", ImVec2(120.0f, 0.0f))) {
            ApplyCanvasResize(m_canvasWidthDraft, m_canvasHeightDraft);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

private:
    bool m_shouldOpenCanvasSizePopup{false};
    int m_canvasWidthDraft{1};
    int m_canvasHeightDraft{1};
};

} // namespace app