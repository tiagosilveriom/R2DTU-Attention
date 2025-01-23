
find_package(PythonLibs REQUIRED)

set(ASR_SRCS ../asr/DanishASR.cpp ../asr/EnglishASR.cpp)
set(REASONING_SRCS ../reasoning/EpistemicReasoning ../reasoning/WorldModel)
set(STATE_SRCS states states/Calibration states/Greet states/Observing)
set(DEMO_SRCS executor ../behaviour/behaviour executor EntityHeadTracker)

add_library(demo SHARED ${DEMO_SRCS} ${REASONING_SRCS} ${ASR_SRCS} ${STATE_SRCS})
set_property(TARGET demo PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(demo common del planner planner ${PYTHON_LIBRARIES})