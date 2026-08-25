// GenericNodeEditor.hpp
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cmath>

namespace app {

using NodeId = int;
using PinId  = int;
using LinkId = int;

enum class PinKind { Input, Output };

struct GraphPinData {
    PinId id{0};
    NodeId node_id{0};
    std::string name;
    PinKind kind{PinKind::Input};
    int type_id{0};
};

struct GraphNodeData {
    NodeId id{0};
    std::string title;
    std::string type_name;
    float position_x{100.0f};
    float position_y{100.0f};
    float parameter_1{0.5f};
    float parameter_2{0.5f};
    bool enabled{true};
    std::vector<GraphPinData> inputs;
    std::vector<GraphPinData> outputs;
};

struct GraphLinkData {
    LinkId id{0};
    PinId start_pin_id{0};
    PinId end_pin_id{0};
};

struct GraphData {
    int version{1};
    std::vector<GraphNodeData> nodes;
    std::vector<GraphLinkData> links;
};

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

struct NodeCreationEntry {
    std::string label;
    std::function<void(ImVec2)> create;
};

struct NodeContextEntry {
    std::string label;
    std::function<void(NodeId)> invoke;
};

class NodeEditorContext {
public:
    ImVec2 scrolling{0.0f, 0.0f};
    ImVec2 canvas_origin{0.0f, 0.0f};
    PinId active_dragging_pin{0};
    PinId hovered_pin{0};
    NodeId selected_node{0};
    NodeId hovered_node{0};
    LinkId hovered_link{0};
    NodeId requested_node_deletion{0};
    bool one_link_per_input{true};
    std::function<void()> on_undo_point;
    std::function<void(NodeId)> on_node_deleted;
    std::vector<NodeContextEntry> node_context_entries;
    
    void ClearHovered() { hovered_pin = 0; }
};

class NodeEditor {
public:
    static Node ToRuntimeNode(const GraphNodeData& data) {
        Node node{data.id, data.title, ImVec2(data.position_x, data.position_y), ImVec2(0, 0), {}, {}, ImColor(60, 60, 70)};
        for (const auto& pin : data.inputs) node.inputs.push_back({pin.id, pin.node_id, pin.name, pin.kind, pin.type_id});
        for (const auto& pin : data.outputs) node.outputs.push_back({pin.id, pin.node_id, pin.name, pin.kind, pin.type_id});
        return node;
    }

    static GraphNodeData ToGraphNode(const Node& node, const std::string& type_name = {}) {
        GraphNodeData data{node.id, node.title, type_name, node.pos.x, node.pos.y};
        for (const auto& pin : node.inputs) data.inputs.push_back({pin.id, pin.node_id, pin.name, pin.kind, pin.type_id});
        for (const auto& pin : node.outputs) data.outputs.push_back({pin.id, pin.node_id, pin.name, pin.kind, pin.type_id});
        return data;
    }

    static Link ToRuntimeLink(const GraphLinkData& data) {
        return {data.id, data.start_pin_id, data.end_pin_id};
    }

    static GraphLinkData ToGraphLink(const Link& link) {
        return {link.id, link.start_pin_id, link.end_pin_id};
    }

    static void BeginCanvas(const char* canvas_id, NodeEditorContext& ctx, ImVec2 size = ImVec2(0, 0)) {
        ImGui::BeginChild(canvas_id, size, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ctx.canvas_origin = canvas_pos;
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
        ctx.hovered_node = 0;
        ctx.hovered_link = 0;
    }

    static void EndCanvas(NodeEditorContext& ctx, std::vector<Node>& nodes, std::vector<Link>& links,
                          const std::unordered_map<PinId, Pin>& pin_map,
                          const std::vector<NodeCreationEntry>& creation_entries,
                          const std::function<void(PinId, PinId)>& on_link_created) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        if (ctx.hovered_link == 0 && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ctx.hovered_link = FindHoveredLink(links);
        }
        if (ctx.hovered_link != 0 && ImGui::BeginPopupContextWindow("##node_editor_link_context", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Delete connection")) {
                if (ctx.on_undo_point) ctx.on_undo_point();
                links.erase(std::remove_if(links.begin(), links.end(), [&ctx](const Link& link) {
                    return link.id == ctx.hovered_link;
                }), links.end());
                ctx.hovered_link = 0;
            }
            ImGui::EndPopup();
        }

        if (ctx.hovered_node == 0 && ctx.hovered_link == 0 &&
            ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("##node_editor_canvas_context");
        }

        if (ImGui::BeginPopup("##node_editor_canvas_context")) {
            if (creation_entries.empty()) {
                ImGui::TextDisabled("No node types available");
            } else {
                for (const auto& entry : creation_entries) {
                    if (ImGui::MenuItem(entry.label.c_str()) && entry.create) {
                        if (ctx.on_undo_point) ctx.on_undo_point();
                        entry.create(ImGui::GetMousePos());
                    }
                }
            }
            ImGui::EndPopup();
        }

        // Handle active cable creation dragging
        if (ctx.active_dragging_pin != 0) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            // Fetch origin pin pos (cached during node render)
            ImVec2 p1 = s_pin_positions[ctx.active_dragging_pin];
            ImVec2 p2 = mouse_pos;
            draw_list->AddBezierCubic(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, IM_COL32(200, 200, 100, 255), 3.0f);

            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                if (ctx.hovered_pin != 0 && ctx.hovered_pin != ctx.active_dragging_pin) {
                    auto start_it = pin_map.find(ctx.active_dragging_pin);
                    auto end_it = pin_map.find(ctx.hovered_pin);
                    if (start_it != pin_map.end() && end_it != pin_map.end()) {
                        PinId output = ctx.active_dragging_pin;
                        PinId input = ctx.hovered_pin;
                        if (start_it->second.kind == PinKind::Input && end_it->second.kind == PinKind::Output) {
                            output = ctx.hovered_pin;
                            input = ctx.active_dragging_pin;
                        }
                        if (start_it->second.node_id != end_it->second.node_id &&
                            start_it->second.type_id == end_it->second.type_id &&
                            ((start_it->second.kind == PinKind::Output && end_it->second.kind == PinKind::Input) ||
                             (start_it->second.kind == PinKind::Input && end_it->second.kind == PinKind::Output))) {
                            if (ctx.on_undo_point) ctx.on_undo_point();
                            if (ctx.one_link_per_input) {
                                links.erase(std::remove_if(links.begin(), links.end(), [input](const Link& link) {
                                    return link.end_pin_id == input;
                                }), links.end());
                            }
                            if (on_link_created) on_link_created(output, input);
                        }
                    }
                }
                ctx.active_dragging_pin = 0;
            }
        }

        if (ctx.requested_node_deletion != 0 ||
            (ctx.selected_node != 0 && ImGui::IsKeyPressed(ImGuiKey_Backspace, false))) {
            const NodeId deleted_node = ctx.requested_node_deletion != 0 ? ctx.requested_node_deletion : ctx.selected_node;
            if (ctx.on_undo_point) ctx.on_undo_point();
            if (ctx.on_node_deleted) ctx.on_node_deleted(deleted_node);
            nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&ctx, &links](const Node& node) {
                if (node.id != (ctx.requested_node_deletion != 0 ? ctx.requested_node_deletion : ctx.selected_node)) return false;
                for (const auto& pin : node.inputs) {
                    links.erase(std::remove_if(links.begin(), links.end(), [&pin](const Link& link) {
                        return link.end_pin_id == pin.id || link.start_pin_id == pin.id;
                    }), links.end());
                }
                for (const auto& pin : node.outputs) {
                    links.erase(std::remove_if(links.begin(), links.end(), [&pin](const Link& link) {
                        return link.end_pin_id == pin.id || link.start_pin_id == pin.id;
                    }), links.end());
                }
                return true;
            }), nodes.end());
            ctx.selected_node = 0;
            ctx.requested_node_deletion = 0;
        }

        ImGui::EndChild();
    }

    static void DrawLinks(const std::vector<Link>& links, const std::unordered_map<PinId, Pin>& pin_map) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        for (const auto& link : links) {
            auto start_it = pin_map.find(link.start_pin_id);
            auto end_it = pin_map.find(link.end_pin_id);
            auto start_pos = s_pin_positions.find(link.start_pin_id);
            auto end_pos = s_pin_positions.find(link.end_pin_id);
            if (start_it != pin_map.end() && end_it != pin_map.end() &&
                start_pos != s_pin_positions.end() && end_pos != s_pin_positions.end()) {
                ImVec2 p1 = start_pos->second;
                ImVec2 p2 = end_pos->second;
                draw_list->AddBezierCubic(p1, ImVec2(p1.x + 50, p1.y), ImVec2(p2.x - 50, p2.y), p2, IM_COL32(100, 200, 255, 255), 3.0f);
            }
        }
    }

    static void DrawNode(NodeEditorContext& ctx, Node& node, const std::function<void(Node&)>& custom_body_ui) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 node_rect_min = ImVec2(ctx.canvas_origin.x + ctx.scrolling.x + node.pos.x,
                                      ctx.canvas_origin.y + ctx.scrolling.y + node.pos.y);

        ImGui::PushID(node.id);
        ImGui::SetCursorScreenPos(node_rect_min);

        if (node.size.x > 0.0f && node.size.y > 0.0f) {
            draw_list->AddRectFilled(node_rect_min, ImVec2(node_rect_min.x + node.size.x, node_rect_min.y + node.size.y), IM_COL32(45, 45, 50, 230), 6.0f);
            draw_list->AddRectFilled(node_rect_min, ImVec2(node_rect_min.x + node.size.x, node_rect_min.y + 24.0f), node.header_color, 6.0f, ImDrawFlags_RoundCornersTop);
        }

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

        // Finish the card after layout so its measured size is retained for the next frame.
        draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(90, 90, 100, 255), 6.0f);

        // Drag node logic
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("##node_drag", ImVec2(node.size.x, 24.0f));
        if (ImGui::IsItemHovered()) ctx.hovered_node = node.id;
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) ctx.selected_node = node.id;
        if (ImGui::BeginPopupContextItem("##node_context")) {
            ctx.selected_node = node.id;
            if (ImGui::MenuItem("Delete node")) {
                ctx.selected_node = node.id;
                ctx.requested_node_deletion = node.id;
            }
            for (const auto& entry : ctx.node_context_entries) {
                if (ImGui::MenuItem(entry.label.c_str()) && entry.invoke) entry.invoke(node.id);
            }
            ImGui::EndPopup();
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            node.pos.x += ImGui::GetIO().MouseDelta.x;
            node.pos.y += ImGui::GetIO().MouseDelta.y;
        }

        ImGui::SetCursorScreenPos(ctx.canvas_origin);

        ImGui::PopID();
    }

private:
    static inline std::unordered_map<PinId, ImVec2> s_pin_positions;

    static LinkId FindHoveredLink(const std::vector<Link>& links) {
        const ImVec2 mouse = ImGui::GetMousePos();
        for (const auto& link : links) {
            auto start = s_pin_positions.find(link.start_pin_id);
            auto end = s_pin_positions.find(link.end_pin_id);
            if (start == s_pin_positions.end() || end == s_pin_positions.end()) continue;
            ImVec2 previous = start->second;
            for (int step = 1; step <= 20; ++step) {
                const float t = static_cast<float>(step) / 20.0f;
                const float inverse = 1.0f - t;
                ImVec2 current = ImVec2(
                    inverse * inverse * inverse * start->second.x + 3.0f * inverse * inverse * t * (start->second.x + 50.0f) + 3.0f * inverse * t * t * (end->second.x - 50.0f) + t * t * t * end->second.x,
                    inverse * inverse * inverse * start->second.y + 3.0f * inverse * inverse * t * start->second.y + 3.0f * inverse * t * t * end->second.y + t * t * t * end->second.y);
                if (DistanceToSegment(mouse, previous, current) < 8.0f) return link.id;
                previous = current;
            }
        }
        return 0;
    }

    static float DistanceToSegment(ImVec2 point, ImVec2 start, ImVec2 end) {
        const ImVec2 delta = ImVec2(end.x - start.x, end.y - start.y);
        const float length_squared = delta.x * delta.x + delta.y * delta.y;
        const float projection = length_squared > 0.0f
            ? std::clamp(((point.x - start.x) * delta.x + (point.y - start.y) * delta.y) / length_squared, 0.0f, 1.0f)
            : 0.0f;
        const ImVec2 closest = ImVec2(start.x + projection * delta.x, start.y + projection * delta.y);
        const ImVec2 difference = ImVec2(point.x - closest.x, point.y - closest.y);
        return std::sqrt(difference.x * difference.x + difference.y * difference.y);
    }

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