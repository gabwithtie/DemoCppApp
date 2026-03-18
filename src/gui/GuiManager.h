#pragma once

#include "main/MenuBar.h"
#include "features/directory/DirectoryBrowser.h"
#include "features/network/NetworkWindow.h"
#include "features/network/ChatWindow.h"

namespace app {
	class GuiManager {
	public:
		struct WindowAssignmentOverride {
			std::vector<GuiWindow*> top_left;
			std::vector<GuiWindow*> top_right;
			std::vector<GuiWindow*> bottom_left;
			std::vector<GuiWindow*> bottom_right;

			std::vector<std::vector<GuiWindow*>> showns = {
			top_left,
			top_right,
			bottom_left,
			bottom_right
			};
			std::vector<GuiWindow*> hiddens;
		};
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
		WindowAssignmentOverride assignmentoverride;

	public:
		GuiManager(WindowAssignmentOverride = {});
		void Draw();
	};
}