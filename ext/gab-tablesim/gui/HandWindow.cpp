#include "HandWindow.h"
#include "graphics/loaders/TextureLoader.h"
#include <imgui.h>

namespace app::gab {
    void HandWindow::DrawSelf() {
        // 1. Get the local player's hand data
        // This automatically creates a default hand if it's the first access
        HandObject myHand = Hands::Get_instance()->GetData();

        ImGui::Text("Your Hand (%d cards)", (int)myHand.cards.size());
        ImGui::Separator();

        // 2. Define Card Dimensions
        const float cardWidth = 100.0f;
        const float cardHeight = 140.0f;
        const ImVec2 cardSize(cardWidth, cardHeight);

        // 3. Setup a Scrolling Child Region for the cards
        // This prevents the window from getting weird if you have 30 cards
        ImGui::BeginChild("CardsRegion", ImVec2(0, cardHeight + 40), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (int i = 0; i < myHand.cards.size(); i++) {
            ImGui::PushID(i);
            std::string cardName = myHand.cards[i];

            // 4. Fetch Texture from Asset Loader
            auto assetData = TextureLoader::GetAssetRuntimeData(cardName);
            ImTextureID texID = (assetData) ? (ImTextureID)(uintptr_t)assetData->texturehandle : 0;

            // 5. Draw the Card Image
            ImGui::BeginGroup();

            // Add a slight tint if the mouse is hovering
            ImVec4 tint = ImVec4(1, 1, 1, 1);
            if (ImGui::IsItemHovered()) tint = ImVec4(0.8f, 0.8f, 1.0f, 1.0f);

            ImGui::ImageButton(("Image" + std::to_string(i)).c_str(), texID, cardSize, ImVec2(0, 0), ImVec2(1, 1), tint);

            if (ImGui::BeginDragDropSource()) {
                // Send the card name as the payload
                ImGui::SetDragDropPayload("DND_CARD_PLAY", cardName.c_str(), cardName.size() + 1);

                // Show a small preview of the card being dragged
                ImGui::Image(texID, ImVec2(50, 70));
                ImGui::Text("%s", cardName.c_str());

                ImGui::EndDragDropSource();
            }

            ImGui::EndGroup();

            // Place next card to the right
            ImGui::SameLine();
            ImGui::PopID();
        }

        ImGui::EndChild();
    }
}