set(DLIB_USE_CUDA false)
set(USE_AVX_INSTRUCTIONS ON CACHE BOOL "Use AVX instructions")
add_subdirectory(${DEPEND_DIR}/dlib dlib_build)

set(OPENPOSE_INCLUDE ${DEPEND_DIR}/openpose/include/)
set(OPENPOSE_LIBRARIES ${DEPEND_DIR}/openpose/build/src/openpose/libopenpose.so)

set(DARKNET_INCLUDE ${DEPEND_DIR}/darknet/include)
set(DARKNET_LIBRARIES ${DEPEND_DIR}/darknet/libdarknet.so)

set(CUDA_INCLUDE /usr/local/cuda/include)
set(CUDA_LIBRARIES /usr/local/cuda/lib64/libcudart.so)

set(SENSORS_SRCS sensors/Realsense sensors/StreamSensor sensors/LocalSensor)
set(DETECTOR_SRCS detectors/skeleton_detection detectors/face_analysis detectors/marker_detection detectors/object_detection)
set(PERCEPTION_SRCS Perception ${SENSORS_SRCS} ${DETECTOR_SRCS} person/person_tracker.cpp person/person_async.cpp)

add_library(perception SHARED ${PERCEPTION_SRCS})
set_property(TARGET perception PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(perception common core ${OpenCV_LIBS} dlib::dlib ${APRILTAG_LIBRARIES} ${OPENPOSE_LIBRARIES} realsense2 gflags ${DARKNET_LIBRARIES} ${CUDA_LIBRARIES} stdc++fs)
target_include_directories(perception SYSTEM PRIVATE ${OPENPOSE_INCLUDE} ${CUDA_INCLUDE})
