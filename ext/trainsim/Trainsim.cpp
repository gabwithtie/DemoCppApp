#include "Trainsim.h"

#include <magic_enum.hpp>
#include "grid/BuildingTypes.h"

#include "grid/behaviors/territory/Territory.h"

#include "grid/behaviors/production/Headquarters.h"

#include "grid/behaviors/entity/BuilderHolder.h"
#include "gui/grid/renderers/BuilderHolderRenderer.h"

#include "grid/behaviors/EmptyBehavior.h"

#include "grid/behaviors/storage/Conveyor.h"
#include "gui/grid/renderers/ConveyorRenderer.h"

#include "grid/behaviors/production/Miner.h"

#include "grid/behaviors/storage/ProductionInput.h"

#include "grid/behaviors/storage/ProductionOutput.h"

#include "grid/behaviors/lines/PowerMain.h"
#include "gui/grid/renderers/LineRenderer.h"

#include "grid/behaviors/lines/PowerTerminal.h"

#include "grid/behaviors/territory/Outpost.h"


namespace Trainsim {
	Trainsim::Trainsim() {
		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::EMPTY)), new EmptyBehavior());
		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(GroundType::TERRITORY)), new Territory());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::HEADQUARTERS)), new Headquarters());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::BUILDER)), new BuilderHolder());
		gridEditor.RegisterRenderer(std::string(magic_enum::enum_name(BuildingType::BUILDER)), new BuilderHolderRenderer());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::MINER)), new Miner());
		
		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::CONVEYOR)), new Conveyor());
		gridEditor.RegisterRenderer(std::string(magic_enum::enum_name(BuildingType::CONVEYOR)), new ConveyorRenderer());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::PRODUCTION_INPUT)), new ProductionInput());
		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::PRODUCTION_OUTPUT)), new ProductionOutput());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::POWER_MAIN)), new PowerMain());
		gridEditor.RegisterRenderer(std::string(magic_enum::enum_name(BuildingType::POWER_MAIN)), new LineRenderer());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::POWER_TERMINAL)), new PowerTerminal());

		gridManager.RegisterBehavior(std::string(magic_enum::enum_name(BuildingType::OUTPOST)), new Outpost());
	}
}