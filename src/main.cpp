#include "window/Window.h"
#include "gui/GuiManager.h"

//Application Specific Includes

int main(int argc, char** argv) {

    // Initialize Window + GUI
    app::Window window = app::Window("GabApp", 1280, 720);

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
