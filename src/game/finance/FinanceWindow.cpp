#include "FinanceWindow.hpp"
#include <imgui.h>
#include <cmath>

namespace app {

    FinanceWindow::FinanceWindow(const FinanceManager& finance) : m_finance(finance) {
        is_open = true;
    }

    void FinanceWindow::DrawSelf() {
        double balance = m_finance.GetBalance();
        bool in_debt = m_finance.IsInDebt();

        // Financial Solvency Status
        if (in_debt) {
            ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "COMPANY STATUS: IN DEBT");
        }
        else {
            ImGui::TextColored(ImVec4(0.25f, 0.9f, 0.25f, 1.0f), "COMPANY STATUS: SOLVENT");
        }

        ImGui::Separator();

        // Capital Display
        ImGui::TextUnformatted("Current Balance:");
        ImGui::SameLine();
        if (in_debt) {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "-$%.2f", std::abs(balance));
        }
        else {
            ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.35f, 1.0f), "$%.2f", balance);
        }

        ImGui::Spacing();

        // Operating Costs
        if (ImGui::CollapsingHeader("Utility Expenses", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            ImGui::Text("Total Electricity Paid: $%.2f", m_finance.GetTotalElectricityCost());
            ImGui::Text("Government Rate: $%.2f / distance unit", m_finance.GetData().electricity_rate_per_unit);
            ImGui::Unindent();
        }

        // Ledger History
        if (ImGui::CollapsingHeader("Ledger Transactions")) {
            const auto& ledger = m_finance.GetData().ledger;
            if (ledger.empty()) {
                ImGui::TextDisabled("No non-operational transactions logged.");
            }
            else {
                if (ImGui::BeginTable("LedgerTable", 2, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Amount", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                    ImGui::TableHeadersRow();

                    for (auto it = ledger.rbegin(); it != ledger.rend(); ++it) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(it->description.c_str());

                        ImGui::TableSetColumnIndex(1);
                        if (it->amount >= 0.0) {
                            ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.35f, 1.0f), "+$%.2f", it->amount);
                        }
                        else {
                            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "-$%.2f", std::abs(it->amount));
                        }
                    }
                    ImGui::EndTable();
                }
            }
        }
    }

} // namespace app