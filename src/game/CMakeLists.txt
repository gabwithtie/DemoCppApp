#SETUP
set(CUR_TARGET_SUBLIBNAME game)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	"map/TerminalMapWindow.cpp"
	"map/MapManager.cpp"

	"train/TrainListWindow.cpp"
	"train/TrainManager.cpp"
	
	"finance/FinanceManager.cpp"
	"finance/FinanceWindow.cpp"
	
	"station/StationViewerWindow.cpp"
	"station/MarketManager.cpp"
	
	"GameSimulation.cpp"
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PRIVATE imgui)
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${imgui_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::app::math)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::app::gui)