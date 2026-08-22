#SETUP
set(CUR_TARGET_SUBLIBNAME math)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	"Vector2.cpp"
	"Vector2Int.cpp"
	"Vector3.cpp"
	"Vector4.cpp"
	"Matrix4.cpp"
	"Quaternion.cpp" "Constants.h"
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#COMPILE DEFINITIONS SETUP
target_compile_definitions(${CUR_TARGET_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC glm::glm)
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${glm_SOURCE_DIR})

target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
