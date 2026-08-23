#SETUP
set(CUR_TARGET_SUBLIBNAME app)
set(CUR_TARGET_NAME gbe_app_${CUR_TARGET_SUBLIBNAME})

set(${CUR_TARGET_NAME}_src
	"App.cpp"

	gui/timeline/TimelineWindow.cpp
	gui/timeline/ClipManager.cpp
	
	gui/clip/ClipEditorWindow.cpp
	gui/clip/NoteManager.cpp
	
	gui/clip/controls/CtrlControls.cpp
	gui/clip/controls/NoteEditingControls.cpp
	gui/clip/controls/NoteSelection.cpp
	gui/clip/controls/NoteChordControls.cpp
	
	gui/instrument/InstrumentWindow.cpp
	
	audio/AudioEngine.cpp
	audio/MiniAudioImpl.cpp
	audio/SoundFontInstrument.cpp
)

add_library(${CUR_TARGET_NAME} STATIC ${${CUR_TARGET_NAME}_src})
add_library(gbe::app::${CUR_TARGET_SUBLIBNAME} ALIAS ${CUR_TARGET_NAME})
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PRIVATE imgui)
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${imgui_SOURCE_DIR})

target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::app::math)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::app::gui)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC gbe::tools)
target_link_libraries(${CUR_TARGET_NAME} PUBLIC tinysoundfont)
target_include_directories(${CUR_TARGET_NAME} PUBLIC ${miniaudio_SOURCE_DIR})