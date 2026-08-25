#include "App.hpp"
#include "ProjectLoader.hpp"
#include "FileDialogue.hpp"

#include <algorithm>
#include <imgui.h>

namespace gsr
{

    App::App() = default;
    App::~App() = default;

    void App::process_input()
    {
        ImGuiIO &io = ImGui::GetIO();

        if (!io.WantTextInput)
        {
            // Quick Save (Ctrl + S)
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
            {
                if (!ProjectLoader::QuickSave())
                {
                    std::string outPath = gbe::FileDialogue::GetFilePath(gbe::FileDialogue::SAVE, "gsrproj");
                    if (!outPath.empty())
                    {
                        ProjectLoader::SaveProject(outPath);
                    }
                }
            }
        }
    }

    bool App::init()
    {
        s_instance = this;

        return true;
    }

    void App::update(float delta_time)
    {
    }

    void App::shutdown()
    {

    }

} // namespace gsr