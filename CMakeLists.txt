#=========================# PROJECT INIT #=========================#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()
# Set CPP version
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)

# Set name to avoid copy pasting it everywhere
set(PROJECT_NAME "DemoCppApp")
project (${PROJECT_NAME})

#=========================# INIT CODE #=========================#

# Add primary source to this project's executable.
add_executable (${PROJECT_NAME} "src/main.cpp" "src/gui/GuiManager.h" "src/gui/GuiManager.cpp")

#=========================# INTERNAL LIBRARIES #=========================#

add_library(app_internal_libs INTERFACE)

# start [CODE]

# end [CODE]

add_subdirectory("src/gui")
target_link_libraries(app_internal_libs INTERFACE app_gui)

# For each folder/library above, make it so that they can #include anything from the root /src folder
get_target_property(LINKED_LIBS app_internal_libs INTERFACE_LINK_LIBRARIES)
foreach(lib ${LINKED_LIBS})
    string(APPEND LINKED_LIBS_STRING " | ${lib} ")
    target_include_directories(${lib} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
endforeach()

# Finally, link all libraries to our main
target_link_libraries(${PROJECT_NAME} PRIVATE app_internal_libs)

#=========================# EXTERNAL LIBRARIES #=========================#
#WINDOW CORE LIBRARY
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
#RENDERER LIBRARY
find_package(GLEW CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
#GUI LIBRARY
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
