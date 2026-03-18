#pragma once

#include "gab-tablesim/network/Decks.h"
#include "gui/main/GuiWindow.h"
#include <string>
#include <vector>

namespace app::gab {
	class DeckWindow : public GuiWindow {
	public:
		std::string GetWindowId() override { return "Decks"; }
	protected:
		void DrawSelf() override;
	private:
		// State for the Inspector Popup
		int m_selectedDeckIdx = -1;
		bool m_openInspector = false;
		int m_carouselIdx = 0; // Moved to member for better state management

		void DrawDeckThumbnail(const char* label, const std::string& topCardName, int index);
		void DrawInspectorPopup();
	};
}