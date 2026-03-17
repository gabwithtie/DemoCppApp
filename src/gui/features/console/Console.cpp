
#include "Console.h"

namespace app {
    Console::Console(AppConsoleRedirector& _redirector) : redirector(_redirector){

    }

	void Console::DrawSelf()
	{
        for (const auto& line : redirector.log_lines) {
            ImGui::TextUnformatted(line.c_str());
        }

        // Auto-scroll logic
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
	}
}