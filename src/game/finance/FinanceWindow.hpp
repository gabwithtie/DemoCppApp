#pragma once

#include "gui/main/GuiWindow.h"
#include "FinanceManager.hpp"

namespace app {

    class FinanceWindow : public GuiWindow {
    public:
        FinanceWindow(const FinanceManager& finance);
        std::string GetWindowId() override { return "Company Finances##app"; }

    protected:
        void DrawSelf() override;

    private:
        const FinanceManager& m_finance;
    };

} // namespace app