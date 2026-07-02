#include "window/Window.h"
#include "gui/GuiManager.h"
#include "gui/features/console/Console.h"
#include "system/AppConsoleRedirector.h"
#include "graphics/loaders/TextureLoader.h"

//Application Specific Includes

int main(int argc, char** argv) {

    AppConsoleRedirector redirector;

    // Initialize Window + GUI
    app::Window window = app::Window("GabApp", 1280, 720);
    app::Console consolewindow = app::Console(redirector);

    //GRAPHICS
    app::TextureLoader textureloader;
    textureloader.AssignSelfAsLoader();


    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN


    //MAIN
    bool done = false;
    while (!done) {
        window.InitFrame();

        if (window.GetShouldQuit())
            break;

        guimanager.Draw();

        window.CommitFrame();
    }

    return 0;
}
