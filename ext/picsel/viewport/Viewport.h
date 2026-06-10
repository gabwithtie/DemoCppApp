#pragma once

#include <string>

namespace picsel {

    class Viewport {
    public:
        Viewport();
        ~Viewport();

        static Viewport* Get() { return s_instance; }

        void SetActiveCanvas(const std::string& physical_png_name);
        std::string GetActiveCanvas() const { return m_active_canvas; }
        bool HasActiveCanvas() const { return !m_active_canvas.empty(); }

        // Core synchronization checks
        bool PullNeedsViewReset();

        // View Navigation Transformations
        void AddPan(float dx, float dy);
        void SetPan(float x, float y);
        void GetPan(float& out_x, float& out_y) const;

        void AddZoom(float dz);
        void SetZoom(float z);
        float GetZoom() const;

    private:
        static Viewport* s_instance;

        std::string m_active_canvas = "";

        float m_pan_x = 0.0f;
        float m_pan_y = 0.0f;
        float m_zoom = 1.0f;

        bool m_needs_view_reset = false;
    };
}