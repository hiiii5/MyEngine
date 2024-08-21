cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(Sandbox)

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# -------------------------------------------
# COMPILER FLAGS/HINTS
# -------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
# -------------------------------------------

include("${CMAKE_SOURCE_DIR}/cmake/find_spdlog.cmake")
find_spdlog()
# -------------------------------------------
# COMPILE/LINKING
# -------------------------------------------
include_directories("${CMAKE_SOURCE_DIR}/MyEngine/src")
link_directories(${CMAKE_BINARY_DIR}/bin)
add_executable(Sandbox "${CMAKE_SOURCE_DIR}/Sandbox/src/SandboxApp.cpp")
# target_include_directories(Sandbox PRIVATE "${CMAKE_SOURCE_DIR}/MyEngine/src")
target_link_libraries(Sandbox PRIVATE MyEngine)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT
                                                              Sandbox)
endif()
# -------------------------------------------
