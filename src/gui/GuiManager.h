#pragma once

#include "main/MenuBar.h"

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
			GuiWindow* startupWindow = nullptr;
		};
	private:
		bool guiStartframeInit = false;
		
		// Main Elements
		MenuBar menuBar;

		std::vector<GuiWindow*> windows = {

		};
		WindowAssignmentOverride assignmentOverride;
		GuiWindow* startupWindow = nullptr;

	public:
		MenuBar& GetMenuBar() { return menuBar; }

		GuiManager(WindowAssignmentOverride = {});
		void Draw();
	};
}