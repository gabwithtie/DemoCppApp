#pragma once

#include "../../../gui/main/GuiWindow.h"
#include "../../App.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include <imgui.h>

namespace gsr {
	class StationWindow : public app::GuiWindow {
	private:
		float zoom_level = 1.0f;
		ImVec2 pan{0.0f, 0.0f};
		std::size_t selected_station = static_cast<std::size_t>(-1);
		int connection_from = 0;
		int connection_to = 0;
		char new_station_id[64] = {};

		static constexpr float station_radius = 28.0f;

		static ImVec2 StationPosition(std::size_t index, std::size_t count) {
			constexpr float pi = 3.14159265358979323846f;
			const float angle = count == 0 ? 0.0f : (2.0f * pi * static_cast<float>(index) / static_cast<float>(count));
			return ImVec2(std::cos(angle) * 180.0f, std::sin(angle) * 130.0f);
		}

		static bool SameConnection(const Model::StationConnection& connection, const std::string& from, const std::string& to) {
			return (connection.from == from && connection.to == to)
				|| (connection.from == to && connection.to == from);
		}

		void AddStation(Model::Project& project) {
			const std::string id = new_station_id[0] == '\0'
				? "Station " + std::to_string(project.stations.size() + 1)
				: std::string(new_station_id);
			if (std::none_of(project.stations.begin(), project.stations.end(), [&](const Model::Station& station) { return station.id == id; })) {
				project.stations.push_back(Model::Station{id});
				new_station_id[0] = '\0';
			}
		}

		void AddConnection(Model::Project& project) {
			if (project.stations.size() < 2 || connection_from < 0 || connection_to < 0
				|| connection_from >= static_cast<int>(project.stations.size())
				|| connection_to >= static_cast<int>(project.stations.size())
				|| connection_from == connection_to) return;

			const auto& from = project.stations[connection_from].id;
			const auto& to = project.stations[connection_to].id;
			if (std::none_of(project.station_connections.begin(), project.station_connections.end(),
				[&](const Model::StationConnection& connection) { return SameConnection(connection, from, to); })) {
				project.station_connections.push_back({from, to});
			}
		}

		void DrawToolbar(Model::Project& project) {
			ImGui::SetNextItemWidth(180.0f);
			ImGui::InputTextWithHint("##NewStation", "Station ID", new_station_id, sizeof(new_station_id));
			ImGui::SameLine();
			if (ImGui::Button("Add station")) AddStation(project);
			ImGui::SameLine();
			ImGui::TextDisabled("%zu station(s), %zu connection(s)", project.stations.size(), project.station_connections.size());

			if (project.stations.size() >= 2) {
				ImGui::SetNextItemWidth(130.0f);
				if (ImGui::BeginCombo("##ConnectionFrom", project.stations[connection_from].id.c_str())) {
					for (int index = 0; index < static_cast<int>(project.stations.size()); ++index) {
						if (ImGui::Selectable(project.stations[index].id.c_str(), connection_from == index)) connection_from = index;
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				ImGui::TextUnformatted("to");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(130.0f);
				if (ImGui::BeginCombo("##ConnectionTo", project.stations[connection_to].id.c_str())) {
					for (int index = 0; index < static_cast<int>(project.stations.size()); ++index) {
						if (ImGui::Selectable(project.stations[index].id.c_str(), connection_to == index)) connection_to = index;
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				if (ImGui::Button("Connect")) AddConnection(project);
			}
		}

		void DrawInspector(const Model::Project& project) {
			if (selected_station >= project.stations.size()) return;
			const Model::Station& station = project.stations[selected_station];
			ImGui::Separator();
			ImGui::Text("Station: %s", station.id.c_str());
			ImGui::Text("Items: %zu", station.items.size());
			ImGui::Text("Passengers: %zu", station.passengers.size());
			ImGui::TextUnformatted("Connections:");
			for (const auto& connection : project.station_connections) {
				if (connection.from == station.id) ImGui::BulletText("%s", connection.to.c_str());
				else if (connection.to == station.id) ImGui::BulletText("%s", connection.from.c_str());
			}
		}

		void DrawMap(Model::Project& project) {
			ImGui::BeginChild("StationMapCanvas", ImVec2(0, -110.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			const ImVec2 origin = ImGui::GetCursorScreenPos();
			const ImVec2 available = ImGui::GetContentRegionAvail();
			const ImVec2 center(origin.x + available.x * 0.5f + pan.x, origin.y + available.y * 0.5f + pan.y);
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0.0f) {
				zoom_level = std::clamp(zoom_level + ImGui::GetIO().MouseWheel * 0.1f, 0.35f, 3.0f);
			}
			if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
				pan.x += ImGui::GetIO().MouseDelta.x;
				pan.y += ImGui::GetIO().MouseDelta.y;
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
			}

			auto position_for = [&](std::size_t index) {
				const ImVec2 position = StationPosition(index, project.stations.size());
				return ImVec2(center.x + position.x * zoom_level, center.y + position.y * zoom_level);
			};
			for (const auto& connection : project.station_connections) {
				auto from = std::find_if(project.stations.begin(), project.stations.end(), [&](const Model::Station& station) { return station.id == connection.from; });
				auto to = std::find_if(project.stations.begin(), project.stations.end(), [&](const Model::Station& station) { return station.id == connection.to; });
				if (from != project.stations.end() && to != project.stations.end()) {
					draw_list->AddLine(position_for(static_cast<std::size_t>(from - project.stations.begin())), position_for(static_cast<std::size_t>(to - project.stations.begin())), IM_COL32(120, 140, 155, 220), 3.0f * zoom_level);
				}
			}
			for (std::size_t index = 0; index < project.stations.size(); ++index) {
				const ImVec2 position = position_for(index);
				const bool selected = selected_station == index;
				draw_list->AddCircleFilled(position, station_radius * zoom_level, selected ? IM_COL32(244, 170, 70, 255) : IM_COL32(70, 150, 190, 255));
				draw_list->AddCircle(position, station_radius * zoom_level, IM_COL32(230, 240, 245, 255), 0, 2.0f * zoom_level);
				const ImVec2 label_size = ImGui::CalcTextSize(project.stations[index].id.c_str());
				draw_list->AddText(ImVec2(position.x - label_size.x * 0.5f, position.y + station_radius * zoom_level + 6.0f), IM_COL32(235, 240, 245, 255), project.stations[index].id.c_str());
				if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
					&& std::hypot(ImGui::GetIO().MousePos.x - position.x, ImGui::GetIO().MousePos.y - position.y) <= station_radius * zoom_level) {
					selected_station = index;
				}
			}
			ImGui::Dummy(ImVec2(std::max(available.x, 500.0f), std::max(available.y, 350.0f)));
			ImGui::EndChild();
		}

	protected:
		void DrawSelf() override {
			Model::Project& project = App::GetInstance().project;
			ImGui::TextUnformatted("STATION MAP");
			ImGui::SameLine();
			if (ImGui::Button("Reset view")) { zoom_level = 1.0f; pan = ImVec2(0.0f, 0.0f); }
			DrawToolbar(project);
			DrawMap(project);
			DrawInspector(project);
		}

	public:
		std::string GetWindowId() override { return "Station Map"; }
	};
}
