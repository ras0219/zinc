cmake_minimum_required (VERSION 2.6)
project (libircclient)

include (CheckIncludeFiles)
include (CheckFunctionExists)
include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb -DENABLE_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

check_function_exists (gethostbyname_r HAVE_GETHOSTBYNAME_R)
check_function_exists (localtime_r HAVE_LOCALTIME_R)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)

option(BUILD_SHARED_LIBS "build as a shared library")
option(ENABLE_THREADS "compile with multithread support" ON)
#option(ENABLE_IPV6 "compile with debug information")
option(ENABLE_OPENSSL "compile with OpenSSL support")

add_library(ircclient src/libircclient.c)

if (ENABLE_THREADS)
  find_package (Threads REQUIRED)
  add_definitions(-DENABLE_THREADS -D_REENTRANT)
  target_link_libraries(ircclient ${CMAKE_THREAD_LIBS_INIT})
endif (ENABLE_THREADS)

if (ENABLE_OPENSSL)
  find_package (OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(ircclient ${OPENSSL_LIBRARIES})
  add_definitions(=DENABLE_SSL)
endif (ENABLE_OPENSSL)

