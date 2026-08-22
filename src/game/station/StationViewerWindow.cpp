#include "StationViewerWindow.hpp"
#include <imgui.h>

namespace app {

    StationViewerWindow::StationViewerWindow(GameSimulation& simulation) : m_sim(simulation) {
        is_open = true;
    }

    void StationViewerWindow::DrawSelf() {
        const std::string& selected_id = m_sim.GetSelectedStationId();
        Station* station = m_sim.GetMapManager().FindStationMutable(selected_id);

        if (!station) {
            ImGui::TextDisabled("Click any station on the Terminal Map to inspect.");
            return;
        }

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Station: %s", station->name.c_str());
        ImGui::TextDisabled("ID: %s | Position: (%.0f, %.0f)", station->id.c_str(), station->position.x, station->position.y);

        float progress = station->regen_timer / station->regen_interval;
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "Market Refresh");

        ImGui::Separator();

        // Drop Target for Selling Items directly into the Station Window
        ImGui::BeginChild("StationDropArea", ImVec2(0, 240.0f), true);

        // Waiting Passengers Table
        if (ImGui::CollapsingHeader("Waiting Passengers", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool has_passengers = false;
            if (ImGui::BeginTable("PassengersTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Destination", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Station ID", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableSetupColumn("Passengers", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableHeadersRow();

                for (const auto& item : station->market) {
                    if (item.item_id.rfind("passenger:", 0) == 0) {
                        has_passengers = true;
                        std::string target_id = item.item_id.substr(10);
                        const Station* target_st = m_sim.GetMapManager().FindStation(target_id);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.5f, 1.0f), "%s", target_st ? target_st->name.c_str() : "Unknown");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextDisabled("%s", target_id.c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%d", item.quantity);
                    }
                }
                ImGui::EndTable();
            }
            if (!has_passengers) ImGui::TextDisabled("No passengers currently waiting.");
        }

        // Cargo Commodity Market
        if (ImGui::CollapsingHeader("Cargo Market (Drop Train Cargo Here to Sell)", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("MarketTable", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Commodity", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Stock", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Buy Price", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Sell Price", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableHeadersRow();

                for (const auto& item : station->market) {
                    if (item.item_id.rfind("passenger:", 0) != 0) {
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Selectable(item.item_id.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);

                        // Drag Source for Purchase
                        if (item.quantity > 0 && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            CargoDragPayload payload{};
                            snprintf(payload.station_id, sizeof(payload.station_id), "%s", station->id.c_str());
                            snprintf(payload.item_id, sizeof(payload.item_id), "%s", item.item_id.c_str());
                            payload.buy_price = item.buy_price;
                            payload.available_quantity = item.quantity;

                            ImGui::SetDragDropPayload("DND_CARGO_ITEM", &payload, sizeof(CargoDragPayload));
                            ImGui::Text("Buying %s ($%.2f/unit)", item.item_id.c_str(), item.buy_price);
                            ImGui::EndDragDropSource();
                        }

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", item.quantity);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "$%.2f", item.buy_price);

                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "$%.2f", item.sell_price);
                    }
                }
                ImGui::EndTable();
            }
        }

        ImGui::EndChild();

        // Drag Target: Train Carriage -> Station Sale
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* raw_payload = ImGui::AcceptDragDropPayload("DND_SELL_CARGO_ITEM")) {
                auto* payload = static_cast<const CargoSellDragPayload*>(raw_payload->Data);

                const Train* train = m_sim.GetTrainManager().FindTrain(payload->train_id);
                bool is_parked = train && train->target_station_id.empty();
                bool is_same_station = train && (train->current_station_id == station->id);

                m_pending_sale.train_id = payload->train_id;
                m_pending_sale.carriage_index = payload->carriage_index;
                m_pending_sale.station_id = station->id;
                m_pending_sale.item_id = payload->item_id;
                m_pending_sale.unit_price = m_sim.GetMarketManager().GetItemSellPriceAtStation(*station, payload->item_id);
                m_pending_sale.same_station_valid = (is_parked && is_same_station);
                m_pending_sale.max_quantity = payload->available_quantity;
                m_pending_sale.selected_quantity = 1;
                m_pending_sale.open_modal = true;
            }
            ImGui::EndDragDropTarget();
        }

        // Sale Confirmation Modal
        if (m_pending_sale.open_modal) {
            ImGui::OpenPopup("Sell Cargo Confirmation##Modal");
            m_pending_sale.open_modal = false;
        }

        if (ImGui::BeginPopupModal("Sell Cargo Confirmation##Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (!m_pending_sale.same_station_valid) {
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "INVALID LOCATION!");
                ImGui::Text("Train %s is not currently parked at station %s.", m_pending_sale.train_id.c_str(), m_pending_sale.station_id.c_str());
                ImGui::Spacing();
                if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            }
            else {
                ImGui::Text("Selling Item: %s", m_pending_sale.item_id.c_str());
                ImGui::Text("Station: %s", station->name.c_str());
                ImGui::Separator();

                ImGui::SliderInt("Quantity to Sell", &m_pending_sale.selected_quantity, 1, m_pending_sale.max_quantity);

                double total_revenue = m_pending_sale.selected_quantity * m_pending_sale.unit_price;
                double current_balance = m_sim.GetFinanceManager().GetBalance();

                ImGui::Text("Unit Sale Price: $%.2f", m_pending_sale.unit_price);
                ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.35f, 1.0f), "Total Computed Revenue: +$%.2f", total_revenue);
                ImGui::Text("Balance After Sale: $%.2f", current_balance + total_revenue);

                ImGui::Spacing();
                ImGui::Separator();

                if (ImGui::Button("Confirm Sale", ImVec2(120, 0))) {
                    m_sim.GetMarketManager().SellCargoToStation(
                        m_pending_sale.train_id,
                        m_pending_sale.carriage_index,
                        m_pending_sale.station_id,
                        m_pending_sale.item_id,
                        m_pending_sale.selected_quantity
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