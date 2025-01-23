#Compiler configuration

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -m32 -static-libstdc++ -D_GLIBCXX_USE_CXX11_ABI=0")
set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -m32")

#Dependencies
#TODO Move into seperate FindPackage files

set(ENET_SEARCH_PATH ${DEPEND_DIR}/enet-x86)
set(ENET_INCLUDE_PATH ${ENET_SEARCH_PATH}/include)
set(ENET_LIBRARIES
        ${ENET_SEARCH_PATH}/.libs/libenet.a)

set(HIREDIS_LIBRARIES ${DEPEND_DIR}/hiredis-x86/libhiredis.a)

set(NAOQI_INCLUDE_PATH ${DEPEND_DIR}/naoqi-sdk/include)

set(NAOQI_LIB_SEARCH_PATH ${DEPEND_DIR}/ctc-linux64-atom)

set(CMAKE_INSTALL_RPATH
        ${NAOQI_LIB_SEARCH_PATH}/boost/lib
        ${NAOQI_LIB_SEARCH_PATH}/libnaoqi/lib
        ${NAOQI_LIB_SEARCH_PATH}/openssl/lib
        ${NAOQI_LIB_SEARCH_PATH}/systemd/lib
        ${NAOQI_LIB_SEARCH_PATH}/icu/lib
        ${NAOQI_LIB_SEARCH_PATH}/zlib/lib
        ${NAOQI_LIB_SEARCH_PATH}/xz_utils/lib
        )

set(NAOQI_LIBRARIES
        ${NAOQI_LIB_SEARCH_PATH}/libnaoqi/lib/libalproxies.so
        ${NAOQI_LIB_SEARCH_PATH}/libnaoqi/lib/libalcommon.so
        ${NAOQI_LIB_SEARCH_PATH}/libnaoqi/lib/libalvalue.so
        ${NAOQI_LIB_SEARCH_PATH}/libqi/lib/libqi.so
        ${NAOQI_LIB_SEARCH_PATH}/bzip2/lib/libbz2.so
        ${NAOQI_LIB_SEARCH_PATH}/tbb/lib/libtbb.so
        ${NAOQI_LIB_SEARCH_PATH}/tbb/lib/libtbbmalloc.so
        ${NAOQI_LIB_SEARCH_PATH}/jpeg/lib/libjpeg.so
        ${NAOQI_LIB_SEARCH_PATH}/png/lib/libpng.so
        ${NAOQI_LIB_SEARCH_PATH}/tiff/lib/libtiff.so
        ${NAOQI_LIB_SEARCH_PATH}/ffmpeg/lib/libavcodec.so
        ${NAOQI_LIB_SEARCH_PATH}/ffmpeg/lib/libavutil.so
        ${NAOQI_LIB_SEARCH_PATH}/ffmpeg/lib/libavdevice.so
        ${NAOQI_LIB_SEARCH_PATH}/ffmpeg/lib/libavfilter.so
        ${NAOQI_LIB_SEARCH_PATH}/ffmpeg/lib/libswscale.so
        ${NAOQI_LIB_SEARCH_PATH}/ffmpeg/lib/libavformat.so
        ${NAOQI_LIB_SEARCH_PATH}/opus/lib/libopus.so
        ${NAOQI_LIB_SEARCH_PATH}/speex/lib/libspeex.so
        ${NAOQI_LIB_SEARCH_PATH}/ogg/lib/libogg.so
        ${NAOQI_LIB_SEARCH_PATH}/libtheora/lib/libtheoradec.so
        ${NAOQI_LIB_SEARCH_PATH}/libtheora/lib/libtheoraenc.so
        ${NAOQI_LIB_SEARCH_PATH}/libtheora/lib/libtheora.so
        ${NAOQI_LIB_SEARCH_PATH}/vo-aacenc/lib/libvo-aacenc.so
        ${NAOQI_LIB_SEARCH_PATH}/vo-amrwbenc/lib/libvo-amrwbenc.so
        ${NAOQI_LIB_SEARCH_PATH}/opencore-amr/lib/libopencore-amrwb.so
        ${NAOQI_LIB_SEARCH_PATH}/opencore-amr/lib/libopencore-amrnb.so
        ${NAOQI_LIB_SEARCH_PATH}/vorbis/lib/libvorbis.so
        ${NAOQI_LIB_SEARCH_PATH}/vorbis/lib/libvorbisenc.so
        ${NAOQI_LIB_SEARCH_PATH}/v4l/lib/libv4l1.so
        ${NAOQI_LIB_SEARCH_PATH}/v4l/lib/libv4l2.so
        ${NAOQI_LIB_SEARCH_PATH}/v4l/lib/libv4lconvert.so
        ${NAOQI_LIB_SEARCH_PATH}/opencv2/lib/libopencv_core.so
        ${NAOQI_LIB_SEARCH_PATH}/opencv2/lib/libopencv_imgproc.so
        ${NAOQI_LIB_SEARCH_PATH}/opencv2/lib/libopencv_contrib.so
        ${NAOQI_LIB_SEARCH_PATH}/opencv2/lib/libopencv_highgui.so
        ${NAOQI_LIB_SEARCH_PATH}/boost/lib/libboost_system.so)


#Architecture
set(CMAKE_POSITION_INDEPENDENT_CODE True)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${DEPEND_DIR})
add_subdirectory(${CMAKE_SOURCE_DIR}/src/core)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/common)
add_executable(onboard ${CMAKE_SOURCE_DIR}/src/onboard_runner.cpp)
target_link_libraries(onboard core common)


#List of modules
add_subdirectory(${CMAKE_SOURCE_DIR}/src/onboard_cameras)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/onboard_nao)
