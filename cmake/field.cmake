#Compiler configuration

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

#Dependencies

set(ENET_SEARCH_PATH ${DEPEND_DIR}/enet-x86-64)
set(ENET_INCLUDE_PATH ${ENET_SEARCH_PATH}/include)
set(ENET_LIBRARIES
        ${ENET_SEARCH_PATH}/.libs/libenet.a)

find_package(OpenCV REQUIRED)
find_package(Boost 1.62 REQUIRED)
include_directories("${Boost_INCLUDE_DIRS}" "/usr/include/python3.6")
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.62 COMPONENTS python-py36)
find_package(PythonLibs 3 REQUIRED)
find_package(SDL2 REQUIRED)

set(HIREDIS_LIBRARIES ${DEPEND_DIR}/hiredis/libhiredis.a)

set(APRILTAG_LIBRARIES ${DEPEND_DIR}/apriltags/lib/libapriltag.so)

#Architecture
set(CMAKE_POSITION_INDEPENDENT_CODE True)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${DEPEND_DIR})
add_subdirectory(${CMAKE_SOURCE_DIR}/src/core)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/common)

set(DEL_DIR "${CMAKE_SOURCE_DIR}/del")
set(DEL_SRC_DIR "${DEL_DIR}/src/del/")
set(DEL_SRCS ${DEL_SRC_DIR}/action.cpp ${DEL_SRC_DIR}/domain.cpp ${DEL_SRC_DIR}/formula.cpp ${DEL_SRC_DIR}/state.cpp ../include/msgs/PerceptionMsgs.h)
add_library(del ${DEL_SRCS})
target_include_directories(del PUBLIC ${DEL_DIR}/include ${DEL_DIR}/src)

set(PLANNER_DIR "${CMAKE_SOURCE_DIR}/Pepper_Planner/Pepper_Planner")
set(PLANNER_SRCS
        ${PLANNER_DIR}/Action.cpp ${PLANNER_DIR}/Action_Event.cpp ${PLANNER_DIR}/Action_Library.cpp ${PLANNER_DIR}/Agent.cpp ${PLANNER_DIR}/Bisimulation_Context.cpp
        ${PLANNER_DIR}/DEL_Interface.cpp ${PLANNER_DIR}/DEL_Operations.cpp ${PLANNER_DIR}/Domain.cpp ${PLANNER_DIR}/Environment_Loader.cpp ${PLANNER_DIR}/Formula_Component.cpp
        ${PLANNER_DIR}/Formula.cpp ${PLANNER_DIR}/General_Action.cpp ${PLANNER_DIR}/Graph.cpp ${PLANNER_DIR}/Interface_DTO.cpp ${PLANNER_DIR}/Misc.cpp
        ${PLANNER_DIR}/Node.cpp ${PLANNER_DIR}/Planner.cpp ${PLANNER_DIR}/Policy.cpp ${PLANNER_DIR}/State.cpp  ${PLANNER_DIR}/World.cpp)
add_library(planner ${PLANNER_SRCS})
target_include_directories(planner PUBLIC ${PLANNER_DIR})

#add_executable(event_runner ${CMAKE_SOURCE_DIR}/src/field_runner.cpp)
#target_link_libraries(event_runner core common)
#target_compile_definitions(event_runner PRIVATE EVENT)

#add_executable(vip_runner ${CMAKE_SOURCE_DIR}/src/field_runner.cpp)
#target_link_libraries(vip_runner core common)
#target_compile_definitions(vip_runner PRIVATE VIP)


#add_executable(questionnaire_runner ${CMAKE_SOURCE_DIR}/src/field_runner.cpp)
#target_link_libraries(questionnaire_runner core common)
#target_compile_definitions(questionnaire_runner PRIVATE QUESTIONNAIRE)

#add_executable(false_belief ${CMAKE_SOURCE_DIR}/src/field_runner.cpp)
#target_link_libraries(false_belief core common)
#target_compile_definitions(false_belief PRIVATE FALSE_BELIEF)

add_executable(false_belief ${CMAKE_SOURCE_DIR}/src/field_runner.cpp)
target_link_libraries(false_belief core common)

add_executable(core_test ${CMAKE_SOURCE_DIR}/src/core_test_runner.cpp)
target_link_libraries(core_test core common)
target_include_directories(core_test PRIVATE ${CMAKE_BINARY_DIR}/src/core)

#List of modules
add_subdirectory(${CMAKE_SOURCE_DIR}/src/mockbot)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/ocs)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/perception)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/executor)

