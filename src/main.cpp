#include <iostream>
#include <chrono>

#include "window/Window.h"
#include "gui/GuiManager.h"
#include "gui/features/console/Console.h"
#include "system/AppConsoleRedirector.h"
#include "asset/assetloading/BatchLoader.h"
#include "graphics/loaders/TextureLoader.h"

//Application Specific Includes
#include "trainsim/Trainsim.h"

int main(int argc, char** argv) {

    AppConsoleRedirector redirector;

    // Initialize Window + GUI
    app::Window window = app::Window("GabApp", 1280, 720);
    app::Console consolewindow = app::Console(redirector);

    //GRAPHICS
    app::TextureLoader textureloader;
    textureloader.AssignSelfAsLoader();

    //GAME SPECIFIC
    app::BatchLoader::LoadAssetsFromDirectory("default/trainsimassets");

	Trainsim::Trainsim trainsim;

    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
		.topLeft = {&trainsim.gridEditor},
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN


    //MAIN
    auto last_time = std::chrono::high_resolution_clock::now();
    bool done = false;
    while (!done) {
        window.InitFrame();

        if (window.GetShouldQuit())
            break;

        guimanager.Draw();

        //MAIN START
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;
        float delta_time = elapsed.count();
        //GAME SPECIFIC MAIN
		trainsim.gridManager.Update(delta_time);

        //MAIN END
        last_time = current_time;
        window.CommitFrame();
    }

    return 0;
}
