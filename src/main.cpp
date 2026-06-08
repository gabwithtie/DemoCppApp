//Additional includes go here
#include "window/Window.h"
#include "gui/GuiManager.h"
#include "gui/features/console/Console.h"
#include "network/Network.h"
#include "system/AppConsoleRedirector.h"
#include "graphics/loaders/TextureLoader.h"

#include "picsel/picsel.h"

#include "picsel/gui/ProjectWindow.h"
#include "picsel/gui/ImageBrowser.h"

int main(int argc, char** argv) {

    AppConsoleRedirector redirector;

    // Initialize Window + GUI
    app::Window window = app::Window("Card Game", 1280, 720);
    app::Console consolewindow = app::Console(redirector);

    //GRAPHICS
    app::TextureLoader textureloader;
    textureloader.AssignSelfAsLoader();

    //SPECIFIC CONSTRUCTOR
	auto projectWindow = new picsel::ProjectWindow();
	auto imageBrowser = new picsel::ImageBrowser();

    //GUI
    app::GuiManager::WindowAssignmentOverride windowoverride = {
        .top_left = {
            imageBrowser
		},
        .hiddens = {
            projectWindow
        }
    };
    app::GuiManager guimanager(windowoverride);

    //GAME SPECIFIC PRE-MAIN
    auto picsel_backend = picsel::Picsel();

    //MAIN
    bool done = false;
    while (!done) {
        window.InitFrame();

        if (window.Get_should_quit())
            break;

        guimanager.Draw();

        window.CommitFrame();
    }

    return 0;
}
