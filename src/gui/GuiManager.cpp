#include "GuiManager.h"
#include "main/GuiWindow.h"


#include <imgui.h>
#include <imgui_internal.h>

namespace app {
	GuiManager::GuiManager(WindowAssignmentOverride additionalwindows) : menuBar (this->windows)
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().MouseDoubleClickMaxDist = 10.0f; // Default is 6.0f

		this->assignmentOverride = additionalwindows;

		for (const auto& shown_list : additionalwindows.showns)
			for (const auto& shown : shown_list)
			{
				if (shown != nullptr)
					this->windows.push_back(shown);
			}

		for (const auto& hidden : additionalwindows.hiddens)
		{
			if (hidden != nullptr)
				this->windows.push_back(hidden);
		}
	}
	void GuiManager::Draw()
	{
		ImGuiID dockspace_id = ImGui::GetID("maindockspace");
		ImGui::DockSpaceOverViewport(dockspace_id);

		if (!guiStartframeInit) {
			guiStartframeInit = true;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_size = viewport->WorkSize;
			ImVec2 work_pos = viewport->WorkPos;

			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);
			ImGui::DockBuilderSetNodePos(dockspace_id, viewport->WorkPos);

			ImGuiID dock_main_id = dockspace_id;
			ImGuiID dock_id_top, dock_id_bottom;
			ImGuiID dock_id_topLeft, dock_id_topRght;
			ImGuiID dock_id_bottomLeft, dock_id_bottomRight;

			dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.4f, NULL, &dock_id_top);
			dock_id_topRght = ImGui::DockBuilderSplitNode(dock_id_top, ImGuiDir_Right, 0.3f, NULL, &dock_id_topLeft);
			dock_id_bottomRight = ImGui::DockBuilderSplitNode(dock_id_bottom, ImGuiDir_Right, 0.3f, NULL, &dock_id_bottomLeft);

			// start [CODE]
			
			for (const auto addwindowhere : this->assignmentOverride.topLeft)
			{
				addwindowhere->SetOpen(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_topLeft);
			}
			for (const auto addwindowhere : this->assignmentOverride.topRght)
			{
				addwindowhere->SetOpen(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_topRght);
			}
			for (const auto addwindowhere : this->assignmentOverride.bottomLeft)
			{
				addwindowhere->SetOpen(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_bottomLeft);
			}
			for (const auto addwindowhere : this->assignmentOverride.bottomRight)
			{
				addwindowhere->SetOpen(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_bottomRight);
			}

			// end [CODE]

			ImGui::DockBuilderFinish(dockspace_id);
		}

		this->menuBar.Draw();

		for (const auto& window : this->windows)
		{
			if (window->IsOpen())
				window->Draw();
		}
	}
}