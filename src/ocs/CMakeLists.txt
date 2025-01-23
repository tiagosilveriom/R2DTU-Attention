include_directories(${OpenCV_INCLUDE_DIRS})

set(OCS_SRCS OperatorControlStation ui/Window ui/RenderContext ui/View ui/TopBar ui/GamepadInput ui/Audio
        CameraView EmptyView TelemetryView ScriptRunnerView FalseBeliefView TerminalView ScriptRunner)

add_library(ocs SHARED ${OCS_SRCS})
set_property(TARGET ocs PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(ocs core common ${SDL2_LIBRARIES} ${OpenCV_LIBS} SDL2_gfx SDL2_ttf SDL2_image)