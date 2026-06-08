#include "TableWindow.h"
#include "imgui_internal.h" // For advanced docking/input if needed

#include "gab-tablesim/network/Decks.h"
#include "gab-tablesim/network/Hands.h"

#include "graphics/loaders/TextureLoader.h"
#include "TableSimMenuExtension.h"

namespace app::gab {

    TableWindow::TableWindow(){
        Reload();
    }

    void TableWindow::Reload() {
        m_rects.clear();
        auto& networkData = Table::Get_instance()->GetData();

        for (const auto& obj : networkData) {
            m_rects.push_back({
                .id = obj.id,
                .pos = ImVec2(obj.position.x, obj.position.y),
                .size = ImVec2(100, 140), // Default card size
                .color = IM_COL32(255, 255, 255, 255)
                });
        }
    }

    void TableWindow::DrawSelf() {
        // --- 1. AUTOMATIC DATA SYNC ---
        auto& networkData = Table::Get_instance()->GetData();

        // If count mismatch, force a full reload
        if (networkData.size() != m_rects.size()) {
            Reload();
        }
        else {
            // Update local positions from network for objects NOT being dragged
            for (size_t i = 0; i < networkData.size(); i++) {
                if ((int)m_rects[i].id != m_draggingRectId) {
                    m_rects[i].pos.x = networkData[i].position.x;
                    m_rects[i].pos.y = networkData[i].position.y;
                }
            }
        }

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // --- CANVAS SETUP ---
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
        if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
        ImVec2 canvas_end = ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y);

        draw_list->AddRectFilled(canvas_pos, canvas_end, IM_COL32(30, 30, 30, 255));

        ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        const bool is_hovered = ImGui::IsItemHovered();
        const bool is_active = ImGui::IsItemActive();

        // --- HELPERS ---
        auto WorldToScreen = [&](ImVec2 world_pos) {
            return ImVec2(canvas_pos.x + m_scrollingOffset.x + (world_pos.x * m_zoomLevel),
                canvas_pos.y + m_scrollingOffset.y + (world_pos.y * m_zoomLevel));
            };
        auto ScreenToWorld = [&](ImVec2 screen_pos) {
            return ImVec2((screen_pos.x - canvas_pos.x - m_scrollingOffset.x) / m_zoomLevel,
                (screen_pos.y - canvas_pos.y - m_scrollingOffset.y) / m_zoomLevel);
            };

        ImVec2 mouse_world_pos = ScreenToWorld(io.MousePos);

        //DRAG DROP TARGET
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DECK_CREATE")) {
                int deckIndex = *(const int*)payload->Data;

                // 1. Get Mouse Position in Screen Space
                ImVec2 mousePos = ImGui::GetIO().MousePos;

                // 3. Request the Host to create the object
                TableObject newDeck;
                newDeck.id = Table::Get_instance()->GetData().size(); // Simple ID generation for now
                newDeck.position = gbe::Vector2(mouse_world_pos.x, mouse_world_pos.y);
                newDeck.object_type = TableObjectType::CARDS;
                newDeck.cardshere = Decks::Get_instance()->GetData()[deckIndex].cardnames;

                Table::CreateObject(newDeck);

                std::cout << "Dropped Deck " << deckIndex << " at " << mouse_world_pos.x << ", " << mouse_world_pos.y << std::endl;
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_CARD_PLAY")) {
                const char* cardname = (const char*)payload->Data;

                // 1. Get Mouse Position in Screen Space
                ImVec2 mousePos = ImGui::GetIO().MousePos;

                // 3. Request the Host to create the object
                TableObject newDeck;
                newDeck.id = Table::Get_instance()->GetData().size(); // Simple ID generation for now
                newDeck.position = gbe::Vector2(mouse_world_pos.x, mouse_world_pos.y);
                newDeck.object_type = TableObjectType::CARDS;
                newDeck.cardshere = { cardname };

                Hands::RemoveOne(cardname);
                Table::CreateObject(newDeck);
            }
            ImGui::EndDragDropTarget();
        }

        // We need to store the world position where the right-click happened
        static ImVec2 context_menu_pos;

        // --- CAPTURING THE RIGHT CLICK ---
        // Check if the canvas (InvisibleButton) was right-clicked
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            context_menu_pos = mouse_world_pos; // Store our logical grid position
            ImGui::OpenPopup("canvas_context");
        }

        // --- CONTEXT MENU UI ---
        if (ImGui::BeginPopup("canvas_context")) {
            ImGui::TextDisabled("Table Actions (%.1f, %.1f)", context_menu_pos.x, context_menu_pos.y);
            ImGui::Separator();

            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Reset View")) {
                    m_scrollingOffset = ImVec2(0, 0);
                    m_zoomLevel = 1.0f;
                }
                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        // --- PAN & ZOOM ---
        if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            m_scrollingOffset.x += io.MouseDelta.x;
            m_scrollingOffset.y += io.MouseDelta.y;
        }
        if (is_hovered && io.MouseWheel != 0.0f) {
            float old_zoom = m_zoomLevel;
            m_zoomLevel *= (io.MouseWheel > 0) ? 1.1f : 0.9f;
            m_zoomLevel = ImClamp(m_zoomLevel, 0.1f, 5.0f);
            m_scrollingOffset.x = io.MousePos.x - canvas_pos.x - (mouse_world_pos.x * m_zoomLevel);
            m_scrollingOffset.y = io.MousePos.y - canvas_pos.y - (mouse_world_pos.y * m_zoomLevel);
        }

        // --- DRAGGING & NETWORK REQUESTS ---
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_draggingRectId = -1;
            m_draggingRectIndex = -1;
            for (int i = (int)m_rects.size() - 1; i >= 0; --i) {
                ImVec2 p_min = WorldToScreen(m_rects[i].pos);
                ImVec2 p_max = ImVec2(p_min.x + (m_rects[i].size.x * m_zoomLevel), p_min.y + (m_rects[i].size.y * m_zoomLevel));

                if (ImGui::IsMouseHoveringRect(p_min, p_max)) {
                    m_draggingRectId = m_rects[i].id;
                    m_draggingRectIndex = i;
                    m_dragStartOffset = ImVec2(mouse_world_pos.x - m_rects[i].pos.x, mouse_world_pos.y - m_rects[i].pos.y);
                    break;
                }
            }
        }

        if (m_draggingRectId != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            // 1. Local Prediction (UI moves immediately)
            m_rects[m_draggingRectIndex].pos.x = mouse_world_pos.x - m_dragStartOffset.x;
            m_rects[m_draggingRectIndex].pos.y = mouse_world_pos.y - m_dragStartOffset.y;

            TableObject olddata = Table::Get_instance()->GetData()[m_draggingRectIndex];

            // 2. Network Sync (Send current position to Host/Server)
            olddata.position = { m_rects[m_draggingRectIndex].pos.x, m_rects[m_draggingRectIndex].pos.y };
            Table::Get_instance()->RequestAction(app::NetActionType::Change, m_draggingRectIndex, olddata);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_draggingRectId = -1;
            m_draggingRectIndex = -1;
        }

        // =========================================================================
        // --- RENDERING ---
        draw_list->PushClipRect(canvas_pos, canvas_end, true);

        // --- 3a. Draw the Grid ---
        const ImU32 grid_color = IM_COL32(200, 200, 200, 40); // Faint white
        const float grid_step = 64.0f;                      // Grid lines every 64 logical units

        // Find the world coordinate of the top-left visible pixel
        ImVec2 visible_world_tl = ScreenToWorld(canvas_pos);
        ImVec2 visible_world_br = ScreenToWorld(canvas_end);

        // Round the start/end points to the nearest grid step
        float grid_start_x = floorf(visible_world_tl.x / grid_step) * grid_step;
        float grid_end_x = ceilf(visible_world_br.x / grid_step) * grid_step;
        float grid_start_y = floorf(visible_world_tl.y / grid_step) * grid_step;
        float grid_end_y = ceilf(visible_world_br.y / grid_step) * grid_step;

        // Draw Vertical Lines
        for (float x = grid_start_x; x <= grid_end_x; x += grid_step)
        {
            ImVec2 p1 = WorldToScreen(ImVec2(x, visible_world_tl.y));
            ImVec2 p2 = WorldToScreen(ImVec2(x, visible_world_br.y));
            draw_list->AddLine(p1, p2, grid_color, 1.0f);
        }
        // Draw Horizontal Lines
        for (float y = grid_start_y; y <= grid_end_y; y += grid_step)
        {
            ImVec2 p1 = WorldToScreen(ImVec2(visible_world_tl.x, y));
            ImVec2 p2 = WorldToScreen(ImVec2(visible_world_br.x, y));
            draw_list->AddLine(p1, p2, grid_color, 1.0f);
        }

        for (size_t i = 0; i < m_rects.size(); i++) {
            const auto& tableObj = networkData[i];
            int cardCount = (int)tableObj.cardshere.size();
            auto& rect = m_rects[i];

            ImVec2 p_min = WorldToScreen(rect.pos);
            ImVec2 p_max = ImVec2(p_min.x + (rect.size.x * m_zoomLevel), p_min.y + (rect.size.y * m_zoomLevel));
            ImVec2 size = ImVec2(p_max.x - p_min.x, p_max.y - p_min.y);

            // --- NEW: TEXTURE LOGIC ---
            std::string textureToDisplay = DECKBACK_FILENAME; // Default fallback

            // If the object has cards and is face up, show the top card
            if (!tableObj.cardshere.empty()) {
                if (tableObj.face_up) {
                    textureToDisplay = tableObj.cardshere.back(); // Show top card
                }
                else {
                    textureToDisplay = DECKBACK_FILENAME; // Or whatever your card back asset is named
                }
            }

            auto assetData = TextureLoader::GetAssetRuntimeData(textureToDisplay);
            ImTextureID texID = (assetData) ? (ImTextureID)(uintptr_t)assetData->texturehandle : 0;

            // Draw the image instead of just a filled rect
            if (texID) {
                draw_list->AddImage(texID, p_min, p_max);
            }
            else {
                // Fallback if texture is missing
                draw_list->AddRectFilled(p_min, p_max, rect.color, 5.0f);
            }
            // --------------------------

            // Existing interaction logic (Invisible Button & Popup)
            ImGui::PushID(i);
            ImGui::SetCursorScreenPos(p_min);
            if (ImGui::InvisibleButton("##hitbox", size)) {
                // Handle click if needed
            }

            if (ImGui::BeginPopupContextItem()) {
                // Add a toggle for Face Up/Down in the menu
                if (ImGui::MenuItem(tableObj.face_up ? "Flip Face Down" : "Flip Face Up")) {
                    TableObject updated = tableObj;
                    updated.face_up = !updated.face_up;
                    Table::Get_instance()->Change((int32_t)i, updated);
                }

                ImGui::Separator();

                if (cardCount > 0) {
                    if (ImGui::MenuItem("Draw 1 to Hand")) {
                        DrawCardsFromDeck(i, 1);
                    }
                    if (cardCount >= 5 && ImGui::MenuItem("Draw 5 to Hand")) {
                        DrawCardsFromDeck(i, 5);
                    }
                }
                else {
                    ImGui::TextDisabled("No cards to draw");
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        draw_list->PopClipRect();

        // PART 4: OVERLAY (DEBUG/INFO)
        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 10));
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Zoom: %.2f (ScrollWheel)", m_zoomLevel);
        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 30));
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Pan: %.0f, %.0f (MiddleMouseDrag)", m_scrollingOffset.x, m_scrollingOffset.y);
        ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x + 10, canvas_pos.y + 50));
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mouse WorldPos: %.1f, %.1f", mouse_world_pos.x, mouse_world_pos.y);
    }

    void TableWindow::DrawCardsFromDeck(int deckIndex, int count)
    {
        // 1. Get the data
        TableObject deck = Table::Get_instance()->GetData()[deckIndex];
        HandObject myHand = Hands::Get_instance()->GetData(); // Gets the local user's hand

        // 2. Transfer the card strings
        for (int i = 0; i < count; i++) {
            if (deck.cardshere.empty()) break;

            // Take from top of deck (end of vector)
            std::string cardToMove = deck.cardshere.back();
            deck.cardshere.pop_back();

            // Add to hand
            myHand.cards.push_back(cardToMove);
        }

        // 3. Sync Table (tells everyone the deck is smaller)
        Table::DrawFrom(deckIndex, deck);

        // 4. Sync Hand (tells everyone you have more cards)
        Hands::Get_instance()->RequestUpdate(myHand);

        std::cout << "Drew " << count << " cards. New hand size: " << myHand.cards.size() << std::endl;
    }
}