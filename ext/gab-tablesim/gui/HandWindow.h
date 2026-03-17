#pragma once

#include "gab-tablesim/network/Hands.h"
#include "gui/main/GuiWindow.h"

namespace app::gab {
	class HandWindow : public GuiWindow {
	public:
		std::string GetWindowId() override { return "Hand"; }
	protected:
		void DrawSelf() override;
	};
}