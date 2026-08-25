#pragma once

#include "../../../model/Train.hpp"
#include <functional>
#include <string>

namespace gsr::train_components {
    struct ComponentCallbacks {
        std::function<bool(std::size_t)> can_remove_carriage;
        std::function<void(std::size_t)> remove_carriage;
        std::function<bool(std::size_t, std::size_t, const std::string&)> can_add_furniture;
        std::function<bool(std::size_t, std::size_t)> can_remove_furniture;
        std::function<void(std::size_t, std::size_t, const std::string&)> add_furniture;
        std::function<void(std::size_t, std::size_t)> remove_furniture;
        std::function<bool(const std::string&)> can_create_carriage;
        std::function<void(const std::string&)> create_carriage;
    };
}
