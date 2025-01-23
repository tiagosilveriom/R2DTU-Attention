
add_library(common STATIC Timer.cpp Message.cpp) # OperatorRequester.cpp
target_link_libraries(common ${CMAKE_THREAD_LIBS_INIT})



