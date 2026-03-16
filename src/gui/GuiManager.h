#pragma once

#include "main/MenuBar.h"
#include "features/directory/DirectoryBrowser.h"
#include "features/network/NetworkWindow.h"
#include "features/network/ChatWindow.h"

namespace app {
	class GuiManager {
	private:
		bool gui_startframe_init = false;
		
		// Main Elements
		MenuBar menubar;

		// Window Elements
		NetworkWindow networkwindow;
		ChatWindow chatwindow;

		std::vector<GuiWindow*> windows = {
			&networkwindow,
			&chatwindow
		};

	public:
		GuiManager();
		void Draw();
	};
}