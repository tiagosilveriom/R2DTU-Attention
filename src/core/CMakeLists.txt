include_directories(${ENET_INCLUDE_PATH} ${CMAKE_CURRENT_BINARY_DIR})

include_directories(${DEPEND_DIR})
set(CORE_SOURCES ModuleLoader.cpp Node.cpp Network.cpp KVStore.cpp)

add_library(core STATIC ${CORE_SOURCES})
target_link_libraries(core PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${ENET_LIBRARIES} ${HIREDIS_LIBRARIES})


add_library(core_test_mod SHARED TestMod.cpp)
set_property(TARGET core_test_mod PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(core_test_mod core common ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} ${ENET_LIBRARIES} )
