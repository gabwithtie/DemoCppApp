set(CURRENT_CMAKE_LIB app_game)

#SUBDIRECTORY LOADING
add_library(${CURRENT_CMAKE_LIB} STATIC
	"train/TerminalMapWindow.cpp"
	"train/TrainListWindow.cpp"
	"finance/FinanceManager.cpp"
	"finance/FinanceWindow.cpp"
	"station/StationViewerWindow.cpp"
	"GameSimulation.cpp"
)


target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE imgui::imgui)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE nfd::nfd)

target_compile_definitions(${CURRENT_CMAKE_LIB} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${CURRENT_CMAKE_LIB} PUBLIC glm::glm)

target_link_libraries(${CURRENT_CMAKE_LIB} PRIVATE glaze::glaze)

target_include_directories(${CURRENT_CMAKE_LIB} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")