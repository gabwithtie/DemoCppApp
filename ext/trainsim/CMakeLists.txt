#================# LIBRARY INIT #================#
set(LIBRARY_NAME "trainsim")

add_library(${LIBRARY_NAME} STATIC
	"Trainsim.cpp"
	
	"grid/GridManager.cpp"
	"gui/grid/GridEditor.h"
	"gui/grid/GridEditor.cpp"

	"grid/behaviors/EmptyBehavior.cpp"
	
	
	
	
	"grid/behaviors/BuildingBehavior.cpp" 
	
	"grid/behaviors/storage/StorageBehavior.cpp"
	
	
 "grid/behaviors/storage/Conveyor.h" "grid/behaviors/storage/Conveyor.cpp"
 "gui/grid/renderers/ConveyorRenderer.cpp" "grid/behaviors/production/ProductionBehavior.h" "grid/behaviors/production/ProductionBehavior.cpp" "grid/behaviors/storage/ProductionOutput.h" "grid/behaviors/storage/ProductionOutput.cpp" "grid/behaviors/storage/ProductionInput.h" "grid/behaviors/storage/ProductionInput.cpp" "grid/behaviors/production/Miner.cpp" "grid/behaviors/production/Headquarters.h" "grid/behaviors/production/Headquarters.cpp" "grid/behaviors/entity/EntityHolder.h" "grid/behaviors/entity/EntityHolder.cpp" "grid/behaviors/entity/BuilderHolder.h" "grid/behaviors/entity/BuilderHolder.cpp" "grid/common/CellBehavior.cpp" "gui/grid/renderers/BuilderHolderRenderer.h" "gui/grid/renderers/BuilderHolderRenderer.cpp" "grid/behaviors/Territory/Territory.h" "grid/behaviors/Territory/Territory.cpp" "grid/behaviors/lines/LineBehavior.h" "grid/behaviors/lines/LineBehavior.cpp" "gui/grid/renderers/LineRenderer.cpp" "grid/behaviors/lines/PowerMain.h" "grid/behaviors/lines/PowerMain.cpp" "grid/behaviors/lines/PowerTerminal.cpp" "grid/behaviors/Territory/Outpost.cpp")

#================# LIBRARY SPECIFIC DEPENDENCIES #================#

target_link_libraries(${LIBRARY_NAME} PRIVATE imgui::imgui)

target_link_libraries(${LIBRARY_NAME} PRIVATE nfd::nfd)

target_compile_definitions(${LIBRARY_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(${LIBRARY_NAME} PUBLIC glm::glm)