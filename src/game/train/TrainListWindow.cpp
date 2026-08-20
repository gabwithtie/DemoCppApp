#include "TrainListWindow.hpp"
#include <imgui.h>
#include <algorithm>

namespace app {

    TrainListWindow::TrainListWindow(GameSimulation& simulation) : m_sim(simulation) {
        is_open = true;
    }

    void TrainListWindow::DrawSelf() {
        const std::string& selected_id = m_sim.GetSelectedTrainId();
        Train* train = m_sim.FindTrainMutable(selected_id);

        if (!train) {
            ImGui::TextDisabled("Click any train on the Terminal Map to inspect.");
            return;
        }

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Train: %s", train->id.c_str());
        ImGui::Text("Base Speed: %.0f km/h", train->speed);

        const Station* target_st = m_sim.FindStation(train->target_station_id);
        const Station* current_st = m_sim.FindStation(train->current_station_id);

        if (target_st) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "En Route To: %s", target_st->name.c_str());

            ImGui::TextUnformatted("Active Path:");
            ImGui::SameLine();
            for (size_t p = 0; p < train->active_path.size(); ++p) {
                if (p > 0) ImGui::TextDisabled("->");
                ImGui::SameLine();
                ImGui::TextUnformatted(train->active_path[p].c_str());
                if (p < train->active_path.size() - 1) ImGui::SameLine();
            }

            char overlay[32];
            snprintf(overlay, sizeof(overlay), "Segment %d/%d", train->current_segment + 1, std::max(1, static_cast<int>(train->active_path.size()) - 1));
            ImGui::ProgressBar(train->t, ImVec2(-1.0f, 0.0f), overlay);
        }
        else {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Stationary At: %s", current_st ? current_st->name.c_str() : "In Transit");
        }

        ImGui::Separator();

        std::string combo_preview = target_st ? target_st->name : "Dispatch to Station...";
        if (ImGui::BeginCombo("Set Destination", combo_preview.c_str())) {
            for (const auto& st : m_sim.GetStations()) {
                if (st.id == train->current_station_id) continue;

                bool is_selected = (st.id == train->target_station_id);
                if (ImGui::Selectable(st.name.c_str(), is_selected)) {
                    m_sim.SetTrainDestination(train->id, st.id);
                }

                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();

        // Carriages with Drag-Source Enabled Inventory
        if (ImGui::CollapsingHeader("Carriages & Cargo Inventory", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (train->carriages.empty()) {
                ImGui::TextDisabled("No carriages attached.");
            }
            else {
                for (size_t c = 0; c < train->carriages.size(); ++c) {
                    auto& carriage = train->carriages[c];

                    ImGui::PushID(static_cast<int>(c));
                    ImGui::Text("Carriage %s", carriage.id.c_str());

                    ImGui::BeginChild("CarriageDropZone", ImVec2(0, 70.0f), true);
                    if (carriage.inventory.empty()) {
                        ImGui::TextDisabled("[Empty Carriage Slot]");
                    }
                    else {
                        for (const auto& item : carriage.inventory) {
                            ImGui::Selectable((item.item_id + " (x" + std::to_string(item.quantity) + ")").c_str());

                            // Drag Source: Train -> Station Sale
                            if (item.quantity > 0 && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                CargoSellDragPayload payload{};
                                snprintf(payload.train_id, sizeof(payload.train_id), "%s", train->id.c_str());
                                payload.carriage_index = c;
                                snprintf(payload.item_id, sizeof(payload.item_id), "%s", item.item_id.c_str());
                                payload.available_quantity = item.quantity;

                                ImGui::SetDragDropPayload("DND_SELL_CARGO_ITEM", &payload, sizeof(CargoSellDragPayload));
                                ImGui::Text("Selling %s (x%d)", item.item_id.c_str(), item.quantity);
                                ImGui::EndDragDropSource();
                            }
                        }
                    }
                    ImGui::EndChild();

                    // Drag Target: Station -> Train Purchase
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* raw_payload = ImGui::AcceptDragDropPayload("DND_CARGO_ITEM")) {
                            auto* payload = static_cast<const CargoDragPayload*>(raw_payload->Data);

                            bool is_parked = train->target_station_id.empty();
                            bool is_same_station = (train->current_station_id == payload->station_id);

                            m_pending_purchase.station_id = payload->station_id;
                            m_pending_purchase.train_id = train->id;
                            m_pending_purchase.carriage_index = c;
                            m_pending_purchase.item_id = payload->item_id;
                            m_pending_purchase.unit_price = payload->buy_price;
                            m_pending_purchase.same_station_valid = (is_parked && is_same_station);

                            if (m_pending_purchase.same_station_valid) {
                                double balance = m_sim.GetFinanceManager().GetBalance();
                                int max_affordable = (payload->buy_price > 0.0) ? static_cast<int>(balance / payload->buy_price) : payload->available_quantity;
                                m_pending_purchase.max_quantity = std::min(payload->available_quantity, max_affordable);
                                m_pending_purchase.selected_quantity = std::min(1, m_pending_purchase.max_quantity);
                            }

                            m_pending_purchase.open_modal = true;
                        }
                        ImGui::EndDragDropTarget();
                    }

                    ImGui::PopID();
                    ImGui::Spacing();
                }
            }
        }

        // Purchase Modal Dialog
        if (m_pending_purchase.open_modal) {
            ImGui::OpenPopup("Cargo Purchase Order##Modal");
            m_pending_purchase.open_modal = false;
        }

        if (ImGui::BeginPopupModal("Cargo Purchase Order##Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (!m_pending_purchase.same_station_valid) {
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "INVALID LOCATION!");
                ImGui::Text("Train %s is not parked at station %s.", m_pending_purchase.train_id.c_str(), m_pending_purchase.station_id.c_str());
                ImGui::Spacing();
                if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            }
            else if (m_pending_purchase.max_quantity <= 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "CANNOT PURCHASE");
                ImGui::Text("Insufficient funds or zero stock available for %s.", m_pending_purchase.item_id.c_str());
                ImGui::Spacing();
                if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            }
            else {
                ImGui::Text("Purchasing: %s", m_pending_purchase.item_id.c_str());
                ImGui::Text("Target Train: %s (Carriage %d)", m_pending_purchase.train_id.c_str(), static_cast<int>(m_pending_purchase.carriage_index + 1));
                ImGui::Separator();

                ImGui::SliderInt("Quantity", &m_pending_purchase.selected_quantity, 1, m_pending_purchase.max_quantity);

                double total_cost = m_pending_purchase.selected_quantity * m_pending_purchase.unit_price;
                double current_balance = m_sim.GetFinanceManager().GetBalance();

                ImGui::Text("Unit Price: $%.2f", m_pending_purchase.unit_price);
                ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.35f, 1.0f), "Total Computed Cost: $%.2f", total_cost);
                ImGui::Text("Balance After Purchase: $%.2f", current_balance - total_cost);

                ImGui::Spacing();
                ImGui::Separator();

                if (ImGui::Button("Confirm Purchase", ImVec2(140, 0))) {
                    m_sim.BuyCargoFromStation(
                        m_pending_purchase.station_id,
                        m_pending_purchase.train_id,
                        m_pending_purchase.carriage_index,
                        m_pending_purchase.item_id,
                        m_pending_purchase.selected_quantity
                    );
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

} // namespace app