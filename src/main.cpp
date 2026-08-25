#include "window/Window.h"
#include "gui/GuiManager.h"

// Application Core
#include "app/App.hpp"

// Application GUI Extensions
#include "gui/MenuBarExtension.hpp"

#include "gui/train/TrainWindow.h"
#include "app/gui/ProjectPicker.h"

#include <imgui.h>

int main(int argc, char** argv) {
    // 1. Initialize Native Window
    app::Window window = app::Window("GabApp", 1280, 720);

    // 2. Initialize Core App Lifecycle & State
    gsr::App app;
    if (!app.init()) {
        return -1;
    }

    // 3. Instantiate GUI Windows
    gsr::MenuBarExtension menuBarExtension;

    gsr::TrainWindow trainWindow;
    gsr::ProjectPicker projectPicker;


    // 4. Configure Layout Assignments via Designated Initializers
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .topLeft = {
            &trainWindow
        },
        .bottomLeft = {
            //&window
        },
        .bottomRight = {
            //&window
        },
        .startupWindow = &projectPicker
    };
    app::GuiManager guimanager(windowoverride);

    guimanager.GetMenuBar().AddMenu(&menuBarExtension);

    // 5. Main Execution Loop
    while (!window.GetShouldQuit()) {
        window.InitFrame();

        // Evaluate transport timing & app calculations
        const float delta_time = ImGui::GetIO().DeltaTime;
        app.process_input();
        app.update(delta_time);
        
        // Render ImGui dockspace and active windows
        guimanager.Draw();


        window.CommitFrame();
    }

    // 6. Cleanup
    app.shutdown();
    return 0;
}