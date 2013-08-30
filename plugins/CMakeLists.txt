cmake_minimum_required (VERSION 2.6)
project (zinc_plugins)

include_directories(include)

#ensure that russian plugin is loaded FIRST. This is a MUST
add_library(russian_plugin SHARED russian/russian_plugin.cpp zinc_plugin_host.cpp)
add_library(sample_plugin SHARED sample/sample_plugin.cpp zinc_plugin_host.cpp)
add_library(irccmd_plugin SHARED irccmd/irccmd_plugin.cpp zinc_plugin_host.cpp)
