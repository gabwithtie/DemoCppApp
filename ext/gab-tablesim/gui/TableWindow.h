#pragma once

#include "gab-tablesim/network/Table.h"
#include "gui/main/GuiWindow.h"
#include <vector>

namespace app::gab {
	class TableWindow : public GuiWindow {
		Table& table;

		struct RectObject {
			uint32_t id;  // Changed to match TableObject
			ImVec2 pos;
			ImVec2 size;
			ImU32 color;
		};

		// Persistent State for prediction and view
		std::vector<RectObject> m_rects;
		ImVec2 m_scrollingOffset = ImVec2(0.0f, 0.0f);
		float  m_zoomLevel = 1.0f;
		int    m_draggingRectId = -1;
		int    m_draggingRectIndex = -1; // Added to track vector index
		ImVec2 m_dragStartOffset = ImVec2(0, 0);

	public:
		TableWindow(Table& table);
		std::string GetWindowId() override { return "Table"; }

		// Pulls all data from Table and overwrites local state
		void Reload();

	protected:
		void DrawSelf() override;
	};
}