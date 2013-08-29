cmake_minimum_required (VERSION 2.6)
project (zinc)

include_directories(include)

set(CMAKE_CXX_FLAGS                "-Wall -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

add_library(sample_plugin SHARED sample/sample_plugin.cpp zinc_plugin_host.cpp)
add_library(irccmd_plugin SHARED irccmd/irccmd_plugin.cpp zinc_plugin_host.cpp)
