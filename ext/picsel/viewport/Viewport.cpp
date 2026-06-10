#include "Viewport.h"
#include <algorithm>

namespace picsel {

    Viewport* Viewport::s_instance = nullptr;

    Viewport::Viewport() {
        if (!s_instance) s_instance = this;
    }

    Viewport::~Viewport() {
        if (s_instance == this) s_instance = nullptr;
    }

    void Viewport::SetActiveCanvas(const std::string& physical_png_name) {
        std::string final_name = "";

        size_t dot_pos = physical_png_name.find('.');
        if (dot_pos != std::string::npos)
            final_name = physical_png_name.substr(0, dot_pos);
        else
            final_name = physical_png_name;

        if (m_active_canvas != final_name) {
            m_active_canvas = final_name;
            m_needs_view_reset = true; // Alert the UI window to compute a new base fit zoom
        }
    }

    bool Viewport::PullNeedsViewReset() {
        bool status = m_needs_view_reset;
        m_needs_view_reset = false; // Reset latch state
        return status;
    }

    void Viewport::AddPan(float dx, float dy) {
        m_pan_x += dx;
        m_pan_y += dy;
    }

    void Viewport::SetPan(float x, float y) {
        m_pan_x = x;
        m_pan_y = y;
    }

    void Viewport::GetPan(float& out_x, float& out_y) const {
        out_x = m_pan_x;
        out_y = m_pan_y;
    }

    void Viewport::AddZoom(float dz) {
        m_zoom += dz;
        m_zoom = std::clamp(m_zoom, 0.05f, 64.0f); // Range supporting high precision pixel-art work
    }

    void Viewport::SetZoom(float z) {
        m_zoom = std::clamp(z, 0.05f, 64.0f);
    }

    float Viewport::GetZoom() const {
        return m_zoom;
    }
}