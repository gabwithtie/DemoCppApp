#pragma once

#include "model/Project.hpp"


#include "../gui/features/propertydrawers/PropertyDrawers.hpp"
#include "SerializationIncludes.hpp"

#include "File/Parser.hpp"
#include <string>
#include <memory>
#include <vector>

namespace gsr {

class App : public gbe::ISerializable {
public:
    App();
    ~App();

    inline static App& GetInstance() {
        return *s_instance;
    }

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    bool init();
    void process_input();
    void update(float delta_time);
    void shutdown();

    gbe::SerializedData Serialize() override {
        gbe::SerializedData data = gbe::ISerializable::Serialize();
        data.serialized_variables["project"] = gbe::Parser::ExportClassStr(project);
        return data;
    }

    void Deserialize(gbe::SerializedData& data) override {
        gbe::ISerializable::Deserialize(data);
        auto it = data.serialized_variables.find("project");
        if (it != data.serialized_variables.end()) {
            gbe::Parser::PopulateClassStr(project, it->second);
        }
    }

public:
    Model::Project project;

    std::string current_filepath;

private:
    inline static App* s_instance{nullptr};
};

} // namespace gsr