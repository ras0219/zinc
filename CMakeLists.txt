cmake_minimum_required (VERSION 2.6)
project (zinc)

add_subdirectory(lib/simplejson)
add_subdirectory(lib/libircclient-1.7)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/build_aux/cmake/")
find_package(ZMQ REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

include_directories(lib/libircclient-1.7/include)
include_directories(lib/cppzmq)
include_directories(lib/simplejson/src)
include_directories(megabrain/megahal-9.1.1)
include_directories(plugins/include)
include_directories(include)

set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")

add_subdirectory(plugins)

add_executable(zinc zinc.cpp ircpp.cpp plugins/zinc_plugin_host.cpp)
target_link_libraries(zinc
	${CMAKE_DL_LIBS}
  ircclient
  json
  ${Boost_FILESYSTEM_LIBRARIES}
  ${Boost_SYSTEM_LIBRARIES}
  ${ZMQ_LIBRARIES})

add_executable(zinc_noirc zinc_noirc.cpp plugins/zinc_plugin_host.cpp)
target_link_libraries(zinc_noirc
	${CMAKE_DL_LIBS}
  json
  ${ZMQ_LIBRARIES})

add_executable(megabrain_bin megabrain/megabrain.cpp megabrain/megahal-9.1.1/megahal.c)
target_link_libraries(megabrain_bin ${ZMQ_LIBRARIES})
