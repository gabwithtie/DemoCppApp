#pragma once

#include "main/MenuBar.h"
#include "features/directory/DirectoryBrowser.h"

namespace app {
	class GuiManager {
	private:
		bool gui_startframe_init = false;
		
		// Main Elements
		MenuBar menubar;

		// Window Elements
		DirectoryBrowser directorybrowser_window;

		std::vector<GuiWindow*> windows = {
			&directorybrowser_window
		};

	public:
		GuiManager();
		void Draw();
	};
}