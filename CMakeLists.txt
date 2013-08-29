cmake_minimum_required (VERSION 2.6)
project (zinc)

add_subdirectory(lib/simplejson)
add_subdirectory(plugins)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/build_aux/cmake/")
find_package(ZMQ REQUIRED)
find_package(BOOST REQUIRED)

include_directories(lib/libircclient-1.7/include)
include_directories(lib/cppzmq)
include_directories(lib/simplejson/src)
include_directories(megabrain/megahal-9.1.1)
include_directories(plugins/include)
include_directories(include)

set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

add_executable(zinc zinc.cpp ircpp.cpp plugins/zinc_plugin_host.cpp)
target_link_libraries(zinc
                      ${CMAKE_CURRENT_SOURCE_DIR}/lib/libircclient-1.7/src/libircclient.a
                      json
                      ${BOOST_LIBRARIES}
                      ${ZMQ_LIBRARIES})


add_executable(megabrain megabrain/megabrain.cpp megabrain/megahal-9.1.1/megahal.c)
target_link_libraries(megabrain ${ZMQ_LIBRARIES})
