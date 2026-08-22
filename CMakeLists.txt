#=========================# PROJECT INIT #=========================#
cmake_minimum_required(VERSION 3.22)

# Set name to avoid copy pasting it everywhere
set(PROJECT_NAME "DemoCppApp")
project(${PROJECT_NAME} LANGUAGES C CXX)

# FETCH CONTENT SETUP
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps")
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE) 

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Set CPP version
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#==============MAIN DEPENDENCIES CONFIG================#
# Force all sub-builds to default to Static PRIOR to FetchContent
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE INTERNAL "")

# SDL Configuration
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL2MIXER_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2IMAGE_VENDORED ON CACHE BOOL "" FORCE)

add_compile_definitions(__PRFCHWINTRIN_H)

# GLEW Configuration
set(GLEW_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_GLEW_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_GLEW_STATIC ON CACHE BOOL "" FORCE)

#==============DECLARE DEPENDENCIES================#
FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.2
    UPDATE_COMMAND "" 
)
FetchContent_Declare(glew
    GIT_REPOSITORY https://github.com/perlmint/glew-cmake
    GIT_TAG glew-cmake-2.2.0-1
    UPDATE_COMMAND "" 
)
FetchContent_Declare(glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG v2.6.9
    UPDATE_COMMAND "" 
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8-docking
    SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/imgui"
    UPDATE_COMMAND "" 
)
FetchContent_Declare(nfd
    GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
    GIT_TAG v1.2.1
    UPDATE_COMMAND "" 
)
FetchContent_Declare(stb GIT_REPOSITORY
    https://github.com/nothings/stb.git
    GIT_TAG f75e8d1cad7d90d72ef7a4661f1b994ef78b4e31
    UPDATE_COMMAND "" 
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    UPDATE_COMMAND "" 
)

#==============POPULATE & MAKE AVAILABLE================#
FetchContent_MakeAvailable(SDL2 glm glaze imgui nfd stb glew)

#=========================# INIT CODE #=========================#
add_executable(${PROJECT_NAME} "src/main.cpp")

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()

#=========================# INTERNAL LIBRARIES #=========================#
add_library(app_internal_libs INTERFACE)
# Modern way to propagate includes globally across an interface group:
target_include_directories(app_internal_libs INTERFACE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext"
)

add_subdirectory("src/gui")
target_link_libraries(app_internal_libs INTERFACE app_gui)

add_subdirectory("src/window")
target_link_libraries(app_internal_libs INTERFACE app_window)

add_subdirectory("src/math")
target_link_libraries(app_internal_libs INTERFACE app_math)

add_subdirectory("src/game")
target_link_libraries(app_internal_libs INTERFACE app_game)

add_subdirectory("src/tools")
target_link_libraries(app_internal_libs INTERFACE gbe::tools)

# Link everything seamlessly
target_link_libraries(${PROJECT_NAME} PRIVATE app_internal_libs)

#=========================# EXTERNAL LIBRARIES #=========================#
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    glew
    imgui
    glaze
)

#=========================# FINAL COPY COMMANDS #=========================#
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD     
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/default"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Copying data files to build directory"
)
