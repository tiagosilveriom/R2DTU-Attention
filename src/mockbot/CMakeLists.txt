add_library(mockbot_recorder SHARED MockBotRecorder)
set_property(TARGET mockbot_recorder PROPERTY POSITION_INDEPENDENT_CODE ON)
include_directories(mockbot_recorder ${OpenCV_INCLUDE_DIRS})
target_link_libraries(mockbot_recorder core common ${OpenCV_LIBS})

add_library(mockbot_player SHARED MockBotPlayer)
set_property(TARGET mockbot_player PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(mockbot_player core common)
