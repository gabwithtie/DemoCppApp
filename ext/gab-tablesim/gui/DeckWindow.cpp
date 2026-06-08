#include "DeckWindow.h"
#include "graphics/loaders/TextureLoader.h"
#include <imgui.h>

namespace app::gab {

    void DeckWindow::DrawSelf() {
        ImGui::Text("Your Decks");
        ImGui::Separator();

        // Grid Settings
        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        float button_size = 100.0f;
        float style_spacing = ImGui::GetStyle().ItemSpacing.x;

        auto& decks = Decks::Get_instance()->GetData();

        for (int i = 0; i < decks.size(); i++) {
            ImGui::PushID(i);

            // We use the "top card" name to get the texture handle
            // In a real deck, this would be myDecks[i].cards[0]
            std::string topCardName = decks[i].cardnames[0];

            DrawDeckThumbnail(decks[i].name.c_str(), topCardName, i);

            // Wrap grid
            float last_button_x2 = ImGui::GetItemRectMax().x;
            float next_button_x2 = last_button_x2 + style_spacing + button_size;
            if (i + 1 < decks.size() && next_button_x2 < window_visible_x2)
                ImGui::SameLine();

            ImGui::PopID();
        }

        DrawInspectorPopup();
    }

    void DeckWindow::DrawDeckThumbnail(const char* label, const std::string& topCardName, int index) {
        auto assetData = TextureLoader::GetAssetRuntimeData(topCardName);
        ImTextureID texID = (assetData) ? (ImTextureID)(uintptr_t)assetData->texturehandle : 0;

        ImGui::BeginGroup();
        if (ImGui::ImageButton("##deck_thumb", texID, ImVec2(100, 140))) {
            m_selectedDeckIdx = index;
            m_openInspector = true;
            m_carouselIdx = 0;
        }

        // --- DRAG AND DROP SOURCE ---
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("DND_DECK_CREATE", &index, sizeof(int));
            ImGui::Text("Drag to Table: %s", label);
            ImGui::Image(texID, ImVec2(50, 70));
            ImGui::EndDragDropSource();
        }

        ImGui::TextUnformatted(label);
        ImGui::EndGroup();
    }

    void app::gab::DeckWindow::DrawInspectorPopup() {
        if (m_openInspector) {
            ImGui::OpenPopup("Deck Inspector");
            m_carouselIdx = 0; // Reset scroll on open
            m_openInspector = false;
        }

        ImGui::SetNextWindowSize(ImVec2(400, 550), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Deck Inspector", NULL, ImGuiWindowFlags_NoScrollbar)) {
            auto& decks = Decks::Get_instance()->GetData();
            auto& cards = decks[m_selectedDeckIdx].cardnames;
            int cardCount = (int)cards.size();

            // --- 1. DISPLAY THE CARD ---
            auto assetData = TextureLoader::GetAssetRuntimeData(cards[m_carouselIdx]);
            ImTextureID texID = (assetData) ? (ImTextureID)(uintptr_t)assetData->texturehandle : 0;

            float availX = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((availX - 250) * 0.5f);
            ImGui::Image(texID, ImVec2(250, 350));

            ImGui::SetCursorPosX((availX - ImGui::CalcTextSize(cards[m_carouselIdx].c_str()).x) * 0.5f);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s (%d/%d)", cards[m_carouselIdx].c_str(), m_carouselIdx + 1, cardCount);

            ImGui::Spacing();
            ImGui::Separator();

            // --- 2. THE AUTO-INDEX SCROLLBAR ---
            // We create a child window that is very wide internally (cardCount * some_width)
            // But the window itself is only as wide as the popup.
            float itemWidth = 20.0f; // Virtual width of "one card" in the scrollbar
            ImGui::TextDisabled("Slide to browse cards:");

            ImGui::BeginChild("ScrollBridge", ImVec2(0, 40), false, ImGuiWindowFlags_HorizontalScrollbar);

            // This dummy cursor defines how long the scrollbar is
            ImGui::SetCursorPosX(cardCount * itemWidth);
            ImGui::Dummy(ImVec2(itemWidth, 1.0f));

            // Check the scroll position and update the index
            float scrollX = ImGui::GetScrollX();
            float maxScrollX = ImGui::GetScrollMaxX();

            if (maxScrollX > 0) {
                float percent = scrollX / maxScrollX;
                m_carouselIdx = (int)(percent * (cardCount - 1));

                // Safety clamp
                if (m_carouselIdx < 0) m_carouselIdx = 0;
                if (m_carouselIdx >= cardCount) m_carouselIdx = cardCount - 1;
            }

            ImGui::EndChild();

            // --- 3. FOOTER ---
            if (ImGui::Button("Close", ImVec2(availX, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}