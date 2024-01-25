cmake_minimum_required(VERSION 3.16.3)

project(dbuscpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(PkgConfig REQUIRED) # Include functions provided by PkgConfig module.

pkg_check_modules(DBUS REQUIRED dbus-1) # This calls pkgconfig with appropriate arguments
# Use results of pkg_check_modules() call.
include_directories(${DBUS_INCLUDE_DIRS})
link_directories(${DBUS_LIBRARY_DIRS})

add_executable(dbuscpp
  ./src/connection.cc 
  main.cpp
)
# boost

target_link_libraries(dbuscpp ${DBUS_LIBRARIES})
set_property(TARGET dbuscpp PROPERTY CXX_STANDARD 17 CXX_O)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -rdynamic")
