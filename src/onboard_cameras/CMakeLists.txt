include_directories(${NAOQI_INCLUDE_PATH})

add_library(onboard_cameras SHARED Cameras.cpp OV5640.cpp)
set_property(TARGET onboard_cameras PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(onboard_cameras ${NAOQI_LIBRARIES} core common)
