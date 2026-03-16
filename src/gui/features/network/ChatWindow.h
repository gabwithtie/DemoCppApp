#pragma once

#include "gui/main/GuiWindow.h"

namespace app {
	class ChatWindow : public GuiWindow {
	public:
		std::string GetWindowId() override { return "Chat"; }
	protected:
		void DrawSelf() override;
	};
}