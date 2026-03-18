#include "GuiManager.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace app {
	GuiManager::GuiManager(WindowAssignmentOverride additionalwindows) : menubar (this->windows)
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		this->assignmentoverride = additionalwindows;

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

		if (!gui_startframe_init) {
			gui_startframe_init = true;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_size = viewport->WorkSize;
			ImVec2 work_pos = viewport->WorkPos;

			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);
			ImGui::DockBuilderSetNodePos(dockspace_id, viewport->WorkPos);

			ImGuiID dock_main_id = dockspace_id;
			ImGuiID dock_id_top, dock_id_bottom;
			ImGuiID dock_id_top_left, dock_id_top_right;
			ImGuiID dock_id_bottom_left, dock_id_bottom_right;

			dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.4f, NULL, &dock_id_top);
			dock_id_top_right = ImGui::DockBuilderSplitNode(dock_id_top, ImGuiDir_Right, 0.3f, NULL, &dock_id_top_left);
			dock_id_bottom_right = ImGui::DockBuilderSplitNode(dock_id_bottom, ImGuiDir_Right, 0.3f, NULL, &dock_id_bottom_left);

			// start [CODE]
			this->networkwindow.Set_is_open(true);
			this->chatwindow.Set_is_open(true);

			ImGui::DockBuilderDockWindow(this->networkwindow.GetWindowId().c_str(), dock_id_top_right);
			ImGui::DockBuilderDockWindow(this->chatwindow.GetWindowId().c_str(), dock_id_bottom_right);

			for (const auto addwindowhere : this->assignmentoverride.top_left)
			{
				addwindowhere->Set_is_open(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_top_left);
			}
			for (const auto addwindowhere : this->assignmentoverride.top_right)
			{
				addwindowhere->Set_is_open(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_top_right);
			}
			for (const auto addwindowhere : this->assignmentoverride.bottom_left)
			{
				addwindowhere->Set_is_open(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_bottom_left);
			}
			for (const auto addwindowhere : this->assignmentoverride.bottom_right)
			{
				addwindowhere->Set_is_open(true);
				ImGui::DockBuilderDockWindow(addwindowhere->GetWindowId().c_str(), dock_id_bottom_right);
			}

			// end [CODE]

			ImGui::DockBuilderFinish(dockspace_id);
		}

		this->menubar.Draw();

		for (const auto& window : this->windows)
		{
			if (window->Get_is_open())
				window->Draw();
		}
	}
}