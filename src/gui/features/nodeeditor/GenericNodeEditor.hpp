// GenericNodeEditor.hpp
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>

namespace app {

using NodeId = int;
using PinId  = int;
using LinkId = int;

enum class PinKind { Input, Output };

struct Pin {
    PinId id{0};
    NodeId node_id{0};
    std::string name;
    PinKind kind{PinKind::Input};
    int type_id{0}; // User-defined data type identifier (e.g., 0=Audio, 1=MIDI, 2=Control)
    ImVec2 screen_pos{0.0f, 0.0f};
};

struct Node {
    NodeId id{0};
    std::string title;
    ImVec2 pos{100.0f, 100.0f};
    ImVec2 size{0.0f, 0.0f};
    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
    ImColor header_color{60, 60, 70, 255};
};

struct Link {
    LinkId id{0};
    PinId start_pin_id{0}; // Output pin
    PinId end_pin_id{0};   // Input pin
};

class NodeEditorContext {
public:
    ImVec2 scrolling{0.0f, 0.0f};
    PinId active_dragging_pin{0};
    PinId hovered_pin{0};
    
    void ClearHovered() { hovered_pin = 0; }
};

class NodeEditor {
public:
    static void BeginCanvas(const char* canvas_id, NodeEditorContext& ctx, ImVec2 size = ImVec2(0, 0)) {
        ImGui::BeginChild(canvas_id, size, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();

        // Canvas Panning via Middle Mouse Button
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
            ctx.scrolling.x += ImGui::GetIO().MouseDelta.x;
            ctx.scrolling.y += ImGui::GetIO().MouseDelta.y;
        }

        // Render Background Grid
        const float GRID_SIZE = 32.0f;
        ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
        for (float x = fmodf(ctx.scrolling.x, GRID_SIZE); x < canvas_size.x; x += GRID_SIZE) {
            draw_list->AddLine(ImVec2(canvas_pos.x + x, canvas_pos.y), ImVec2(canvas_pos.x + x, canvas_pos.y + canvas_size.y), GRID_COLOR);
        }
        for (float y = fmodf(ctx.scrolling.y, GRID_SIZE); y < canvas_size.y; y += GRID_SIZE) {
            draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + y), GRID_COLOR);
        }

        ctx.ClearHovered();
    }

    static void EndCanvas(NodeEditorContext& ctx, std::vector<Link>& links, const std::function<void(PinId, PinId)>& on_link_created) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Handle active cable creation dragging
        if (ctx.active_dragging_pin != 0) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            // Fetch origin pin pos (cached during node render)
            ImVec2 p1 = s_pin_positions[ctx.active_dragging_pin];
            ImVec2 p2 = mouse_pos;
            draw_list->AddBezierCubic(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, IM_COL32(200, 200, 100, 255), 3.0f);

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                if (ctx.hovered_pin != 0 && ctx.hovered_pin != ctx.active_dragging_pin) {
                    if (on_link_created) {
                        on_link_created(ctx.active_dragging_pin, ctx.hovered_pin);
                    }
                }
                ctx.active_dragging_pin = 0;
            }
        }

        ImGui::EndChild();
    }

    static void DrawLinks(const std::vector<Link>& links, const std::unordered_map<PinId, Pin>& pin_map) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        for (const auto& link : links) {
            auto start_it = pin_map.find(link.start_pin_id);
            auto end_it = pin_map.find(link.end_pin_id);
            if (start_it != pin_map.end() && end_it != pin_map.end()) {
                ImVec2 p1 = start_it->second.screen_pos;
                ImVec2 p2 = end_it->second.screen_pos;
                draw_list->AddBezierCubic(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, IM_COL32(100, 200, 255, 255), 3.0f);
            }
        }
    }

    static void DrawNode(NodeEditorContext& ctx, Node& node, const std::function<void(Node&)>& custom_body_ui) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
        ImVec2 node_rect_min = ImVec2(canvas_origin.x + ctx.scrolling.x + node.pos.x, canvas_origin.y + ctx.scrolling.y + node.pos.y);

        ImGui::PushID(node.id);
        ImGui::SetCursorScreenPos(node_rect_min);

        ImGui::BeginGroup();
        
        // Node Header
        ImGui::TextUnformatted(node.title.c_str());
        ImGui::Spacing();

        // Render Input & Output Columns
        ImGui::BeginGroup();
        for (auto& pin : node.inputs) {
            DrawPin(ctx, pin, true);
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 20.0f);

        ImGui::BeginGroup();
        if (custom_body_ui) {
            custom_body_ui(node);
        }
        ImGui::EndGroup();

        ImGui::SameLine(0.0f, 20.0f);

        ImGui::BeginGroup();
        for (auto& pin : node.outputs) {
            DrawPin(ctx, pin, false);
        }
        ImGui::EndGroup();

        ImGui::EndGroup();

        // Canvas drag/movement interaction for nodes
        ImVec2 node_rect_max = ImGui::GetItemRectMax();
        node.size = ImVec2(node_rect_max.x - node_rect_min.x, node_rect_max.y - node_rect_min.y);

        // Render Background Card
        draw_list->ChannelsSetCurrent(0);
        draw_list->AddRectFilled(node_rect_min, node_rect_max, IM_COL32(45, 45, 50, 230), 6.0f);
        draw_list->AddRectFilled(node_rect_min, ImVec2(node_rect_max.x, node_rect_min.y + 24.0f), node.header_color, 6.0f, ImDrawFlags_RoundCornersTop);
        draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(90, 90, 100, 255), 6.0f);

        // Drag node logic
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("##node_drag", ImVec2(node.size.x, 24.0f));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            node.pos.x += ImGui::GetIO().MouseDelta.x;
            node.pos.y += ImGui::GetIO().MouseDelta.y;
        }

        ImGui::PopID();
    }

private:
    static inline std::unordered_map<PinId, ImVec2> s_pin_positions;

    static void DrawPin(NodeEditorContext& ctx, Pin& pin, bool is_input) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        float radius = 6.0f;
        ImVec2 pin_center = ImVec2(cursor.x + radius, cursor.y + radius + 2.0f);

        pin.screen_pos = pin_center;
        s_pin_positions[pin.id] = pin_center;

        ImGui::InvisibleButton((std::string("##pin_") + std::to_string(pin.id)).c_str(), ImVec2(radius * 2, radius * 2));

        if (ImGui::IsItemHovered()) {
            ctx.hovered_pin = pin.id;
            draw_list->AddCircleFilled(pin_center, radius + 2.0f, IM_COL32(255, 255, 255, 200));
        } else {
            draw_list->AddCircleFilled(pin_center, radius, IM_COL32(0, 200, 120, 255));
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            ctx.active_dragging_pin = pin.id;
        }

        ImGui::SameLine();
        ImGui::TextUnformatted(pin.name.c_str());
    }
};

} // namespace app