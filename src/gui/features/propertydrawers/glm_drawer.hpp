#pragma once

#include <imgui.h>
#include "PropertyDrawer.hpp"
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp> // Complex types
#include <glm/gtc/type_ptr.hpp>

namespace gbe {


    // COMPLEX TYPE: GLM::VEC3
    template <>
    struct PropertyDrawer<glm::vec3> {
        static bool Draw(const std::string& label, glm::vec3& target) {
             ImGui::DragFloat3(label.c_str(), &target.x, 0.1f);

             return true;
        }
    };

    template <>
    struct PropertyDrawer<glm::vec2> {
        static bool Draw(const std::string& label, glm::vec2& target) {
            ImGui::DragFloat2(label.c_str(), &target.x, 0.1f);

            return true;
        }
    };

    template <>
    struct PropertyDrawer<glm::vec4> {
        static bool Draw(const std::string& label, glm::vec4& target) {
            ImGui::DragFloat4(label.c_str(), &target.x, 0.1f);

            return true;
        }
    };
}