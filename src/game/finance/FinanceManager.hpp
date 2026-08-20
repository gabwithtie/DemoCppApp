#pragma once

#include "FinanceModel.hpp"

namespace app {

    class FinanceManager {
    public:
        FinanceManager(double starting_capital = 100000.0);

        void DeductElectricityCost(double distance_traveled);
        void AddIncome(double amount, const std::string& description, float game_time = 0.0f);
        void AddExpense(double amount, const std::string& description, float game_time = 0.0f);

        double GetBalance() const { return m_data.balance; }
        double GetTotalElectricityCost() const { return m_data.total_electricity_cost; }
        bool IsInDebt() const { return m_data.balance < 0.0; }
        const FinanceData& GetData() const { return m_data; }

    private:
        FinanceData m_data;
    };

} // namespace app