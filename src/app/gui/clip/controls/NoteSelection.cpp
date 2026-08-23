#include "NoteSelection.hpp"
#include "gui/clip/NoteManager.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace gsr::gui {

void NoteSelection::HandleKeyboardShortcuts(gsr::App& /*app*/, Model::Clip& clip) {
    ImGuiIO& io = ImGui::GetIO();

    // Select All (Ctrl + A)
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        for (auto& note : clip.notes) {
            note.selected = true;
        }
        return;
    }

    // --- 0. HOTKEY GUARD ---
    // Ignore selection controls if other modifiers (Ctrl, Alt, Super) are held
    if (io.KeyCtrl || io.KeyAlt || io.KeySuper) return;

    // Ignore Shift + Arrow selection if ANY letter or number key is held down 
    // (e.g. Shift + S + Arrow, Shift + T + Arrow, Shift + G + Arrow)
    for (int k = ImGuiKey_A; k <= ImGuiKey_Z; ++k) {
        if (ImGui::IsKeyDown(static_cast<ImGuiKey>(k))) return;
    }
    for (int k = ImGuiKey_0; k <= ImGuiKey_9; ++k) {
        if (ImGui::IsKeyDown(static_cast<ImGuiKey>(k))) return;
    }

    bool up    = ImGui::IsKeyPressed(ImGuiKey_UpArrow);
    bool down  = ImGui::IsKeyPressed(ImGuiKey_DownArrow);
    bool left  = ImGui::IsKeyPressed(ImGuiKey_LeftArrow);
    bool right = ImGui::IsKeyPressed(ImGuiKey_RightArrow);

    if (!up && !down && !left && !right) return;
    if (clip.notes.empty()) return;

    const bool is_shift = io.KeyShift;

    // 1. Gather current selection
    std::vector<Model::Note*> selected;
    for (auto& note : clip.notes) {
        if (note.selected) {
            selected.push_back(&note);
        }
    }

    if (selected.empty()) {
        auto first_it = std::min_element(clip.notes.begin(), clip.notes.end(),
            [](const Model::Note& a, const Model::Note& b) {
                return a.start_tick < b.start_tick;
            });
        first_it->selected = true;
        return;
    }

    // 2. Map coordinates into Grid-Normalized 2D Space (X = Grid Units, Y = Semitones)
    constexpr uint64_t TICKS_PER_GRID = 480; // Standard grid tick resolution
    auto GetX = [](const Model::Note* n) -> double {
        return static_cast<double>(n->start_tick) / static_cast<double>(TICKS_PER_GRID);
    };
    auto GetY = [](const Model::Note* n) -> double {
        return static_cast<double>(n->pitch);
    };

    // Calculate current selection bounding box & centroid
    double min_x = std::numeric_limits<double>::max();
    double max_x = -std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_y = -std::numeric_limits<double>::max();
    double sum_x = 0.0, sum_y = 0.0;

    for (const auto* n : selected) {
        double x = GetX(n);
        double y = GetY(n);
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
        sum_x += x;
        sum_y += y;
    }
    double center_x = sum_x / selected.size();
    double center_y = sum_y / selected.size();

    // Hysteresis margin (0.15 grid steps) for time-slice matching
    constexpr double SLICE_EPSILON = 0.15;

    Model::Note* best_note = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    // 3. Weighted Directional Euclidean Search
    for (auto& note : clip.notes) {
        double nx = GetX(&note);
        double ny = GetY(&note);

        bool valid_candidate = false;
        double dx = 0.0, dy = 0.0;

        if (up) {
            if (ny > max_y + 0.01) {
                valid_candidate = true;
                dx = (nx - center_x) * 2.5; // Heavily penalize horizontal drift
                dy = ny - max_y;
            }
        } else if (down) {
            if (ny < min_y - 0.01) {
                valid_candidate = true;
                dx = (nx - center_x) * 2.5;
                dy = min_y - ny;
            }
        } else if (right) {
            if (nx > max_x + SLICE_EPSILON) {
                valid_candidate = true;
                dx = nx - max_x;
                dy = (ny - center_y) * 0.5;
            }
        } else if (left) {
            if (nx < min_x - SLICE_EPSILON) {
                valid_candidate = true;
                dx = min_x - nx;
                dy = (ny - center_y) * 0.5;
            }
        }

        if (valid_candidate) {
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < min_distance) {
                min_distance = dist;
                best_note = &note;
            }
        }
    }

    if (!best_note) return;

    // 4. Determine target notes (Shift Range Expansion vs. Single Select)
    std::vector<Model::Note*> to_select;
    to_select.push_back(best_note);

    if (is_shift) {
        double target_x = GetX(best_note);
        double target_y = GetY(best_note);

        if (left || right) {
            // Horizontal expansion: Select target slice notes that strictly match the pitch of any selected note
            for (auto& note : clip.notes) {
                if (&note == best_note) continue;
                double nx = GetX(&note);
                double ny = GetY(&note);

                if (std::abs(nx - target_x) <= SLICE_EPSILON) {
                    bool matches_pitch = false;
                    for (const auto* s : selected) {
                        if (std::abs(ny - GetY(s)) < 0.01) {
                            matches_pitch = true;
                            break;
                        }
                    }
                    if (matches_pitch) {
                        to_select.push_back(&note);
                    }
                }
            }
        } else if (up || down) {
            // Vertical expansion: Select notes at target pitch bounded strictly by selection's horizontal footprint
            for (auto& note : clip.notes) {
                if (&note == best_note) continue;
                double nx = GetX(&note);
                double ny = GetY(&note);

                if (std::abs(ny - target_y) < 0.01 && 
                    nx >= (min_x - SLICE_EPSILON) && 
                    nx <= (max_x + SLICE_EPSILON)) {
                    to_select.push_back(&note);
                }
            }
        }
    }

    // 5. Apply selection
    if (!is_shift) {
        for (auto& note : clip.notes) {
            note.selected = false;
        }
    }
    for (auto* n : to_select) {
        n->selected = true;
    }
}

} // namespace gsr::gui