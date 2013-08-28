cmake_minimum_required (VERSION 2.6)
project (zinc)

add_subdirectory(lib/simplejson)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build_aux/cmake/")
find_package(ZMQ REQUIRED)

include_directories(lib/libircclient-1.7/include)
include_directories(megabrain/megahal-9.1.1)
include_directories(lib/cppzmq)

set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

add_executable(zinc zinc.cpp)
target_link_libraries(zinc
                      ${CMAKE_CURRENT_SOURCE_DIR}/lib/libircclient-1.7/src/libircclient.a
                      ${ZMQ_LIBRARIES})


add_executable(megabrain megabrain/megabrain.cpp megahal-9.1.1/megahal.c)
target_link_libraries(megabrain ${ZMQ_LIBRARIES})
