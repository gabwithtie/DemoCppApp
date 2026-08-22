#SETUP
set(CUR_TARGET_SUBLIBNAME window)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	"Window.cpp"
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#WINDOW CORE LIBRARY
target_link_libraries(${CUR_TARGET_NAME} PRIVATE SDL2-static)
#RENDERER LIBRARY
target_link_libraries(${CUR_TARGET_NAME} PRIVATE libglew_static)
#GUI LIBRARY
target_link_libraries(${CUR_TARGET_NAME} PRIVATE imgui)
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${imgui_SOURCE_DIR})

target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})