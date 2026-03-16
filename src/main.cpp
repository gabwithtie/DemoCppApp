//Additional includes go here
#include "window/Window.h"
#include "gui/GuiManager.h"
#include "network/Network.h"

int main(int argc, char** argv) {

    // Initialize Window + GUI
    const char* window_name = "ImGui App";
    int w = 1280;
    int h = 720;
    app::Window window = app::Window("Card Game", w, h);
    
    app::GuiManager guimanager;
    app::Network network;

    bool done = false;

    while (!done) {
        network.Update();

        window.InitFrame();

        if (window.Get_should_quit())
            break;

        guimanager.Draw();

        window.CommitFrame();
    }

    return 0;
}
