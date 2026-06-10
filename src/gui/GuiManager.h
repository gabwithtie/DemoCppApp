#pragma once

#include "main/MenuBar.h"
#include "features/directory/DirectoryBrowser.h"
#include "features/network/NetworkWindow.h"
#include "features/network/ChatWindow.h"

namespace app {
	class GuiManager {
	public:
		struct WindowAssignmentOverride {
			std::vector<GuiWindow*> topLeft;
			std::vector<GuiWindow*> topRght;
			std::vector<GuiWindow*> bottomLeft;
			std::vector<GuiWindow*> bottomRight;

			std::vector<std::vector<GuiWindow*>> showns = {
			topLeft,
			topRght,
			bottomLeft,
			bottomRight
			};
			std::vector<GuiWindow*> hiddens;
		};
	private:
		bool guiStartframeInit = false;
		
		// Main Elements
		MenuBar menuBar;

		// Window Elements

		std::vector<GuiWindow*> windows = {
		};
		WindowAssignmentOverride assignmentOverride;

	public:
		GuiManager(WindowAssignmentOverride = {});
		void Draw();
	};
}