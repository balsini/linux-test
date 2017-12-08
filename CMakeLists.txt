cmake_minimum_required(VERSION 3.1)
project(linux-test)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-D__DEBUG__)
  message("Debug added as a compilation flag")
endif()

set(LIBRARY_TYPE STATIC)

enable_testing (true)

# Include dirs.
add_subdirectory (src)
