#pragma once

#include <string>
#include <vector>

namespace app {

    struct Transaction {
        std::string description;
        double amount = 0.0; // Positive for income, negative for expenses
        float game_time = 0.0f;
    };

    struct FinanceData {
        double balance = 100000.0;                 // Starting capital ($100,000.00)
        double total_electricity_cost = 0.0;
        double electricity_rate_per_unit = 0.25;   // $0.25 per distance unit traveled
        std::vector<Transaction> ledger;
    };

} // namespace app