#include "FinanceManager.hpp"

namespace app {

    FinanceManager::FinanceManager(double starting_capital) {
        m_data.balance = starting_capital;
    }

    void FinanceManager::DeductElectricityCost(double distance_traveled) {
        if (distance_traveled <= 0.0) return;

        double cost = distance_traveled * m_data.electricity_rate_per_unit;
        m_data.balance -= cost;
        m_data.total_electricity_cost += cost;
    }

    void FinanceManager::AddIncome(double amount, const std::string& description, float game_time) {
        m_data.balance += amount;
        m_data.ledger.push_back({ description, amount, game_time });
    }

    void FinanceManager::AddExpense(double amount, const std::string& description, float game_time) {
        m_data.balance -= amount;
        m_data.ledger.push_back({ description, -amount, game_time });
    }

} // namespace app