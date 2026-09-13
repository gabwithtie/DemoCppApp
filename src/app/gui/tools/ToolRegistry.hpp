#pragma once

#include "Tool.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace app {

class ToolRegistry {
public:
    using ToolFactory = std::function<std::unique_ptr<Tool>()>;

    struct RegistryEntry {
        std::string name;
        ToolFactory factory;
    };

    static std::vector<RegistryEntry>& GetEntries() {
        static std::vector<RegistryEntry> entries;
        return entries;
    }

    static void Register(const std::string& name, ToolFactory factory) {
        for (auto& entry : GetEntries()) {
            if (entry.name == name) {
                entry.factory = std::move(factory);
                return;
            }
        }
        GetEntries().push_back({name, std::move(factory)});
    }

    static std::vector<std::unique_ptr<Tool>> CreateAll() {
        std::vector<std::unique_ptr<Tool>> tools;
        tools.reserve(GetEntries().size());

        for (const auto& entry : GetEntries()) {
            if (entry.factory) {
                tools.push_back(entry.factory());
            }
        }

        return tools;
    }
};

#define GBE_REGISTER_TOOL(Type) \
    inline static bool Type##_tool_registered = []() { \
        app::ToolRegistry::Register(#Type, []() { \
            return std::make_unique<Type>(); \
        }); \
        return true; \
    }()

} // namespace app
