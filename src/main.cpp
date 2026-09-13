#include "window/Window.h"
#include "gui/GuiManager.h"

// Application Core
#include "app/App.hpp"

// Application GUI Extensions
#include "gui/MenuBarExtensionMain.hpp"

#include "app/gui/ProjectPicker.hpp"
#include "app/gui/SampleWindow.hpp"

#include <imgui.h>

int main(int argc, char** argv) {
    // 1. Initialize Native Window
    app::Window window = app::Window("GabApp", 1280, 720);

    // 2. Initialize Core App Lifecycle & State
    app::App app;
    if (!app.init()) {
        return -1;
    }

    // 3. Instantiate GUI Windows
    app::MenuBarExtensionMain menuBarExtensionMain;

    app::ProjectPicker projectPicker;
    app::SampleWindow sampleWindow;


    // 4. Configure Layout Assignments via Designated Initializers
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .topLeft = {
            &sampleWindow
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

    guimanager.GetMenuBar().AddMenu(&menuBarExtensionMain);

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