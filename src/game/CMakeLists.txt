#SETUP
set(CUR_TARGET_SUBLIBNAME game)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	"test.cpp"
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})