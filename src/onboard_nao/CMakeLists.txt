include_directories(${NAOQI_INCLUDE_PATH})

add_library(onboard_nao SHARED nao.cpp)
set_property(TARGET onboard_nao PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(onboard_nao ${NAOQI_LIBRARIES} common)