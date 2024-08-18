cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(MyEngine)

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# -------------------------------------------
# BUNDLE PACKAGES BY PLATFORM
# -------------------------------------------
if (LINUX)
    find_package(Threads REQUIRED)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/find_glm.cmake)
FIND_GLM()

include(${CMAKE_SOURCE_DIR}/cmake/find_vulkan.cmake)
FIND_VULKAN()

include(${CMAKE_SOURCE_DIR}/cmake/find_sdl.cmake)
FIND_SDL()

include(${CMAKE_SOURCE_DIR}/cmake/find_fmt.cmake)
FIND_FMT()
# -------------------------------------------

# -------------------------------------------
# COMPILER FLAGS/HINTS
# -------------------------------------------
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# -------------------------------------------

# -------------------------------------------
# GLSL TO SPIRV
# -------------------------------------------
include(${CMAKE_SOURCE_DIR}/cmake/compile_shaders.cmake)
COMPILE_SHADERS()
# -------------------------------------------

# -------------------------------------------
# COMPILE/LINKING
# -------------------------------------------
file(GLOB SOURCE "src/MyEngine/*.cpp")

if (WIN32)
elseif(APPLE)
    link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
else (WIN32)
    link_libraries(${Vulkan_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
endif (WIN32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

add_library(MyEngine SHARED ${SOURCE})
add_dependencies(MyEngine Shaders)
#include_directories("${CMAKE_SOURCE_DIR}/src/MyEngine")
target_link_libraries(MyEngine PRIVATE SDL2::SDL2 fmt::fmt-header-only)

project(Sandbox)
link_directories(${CMAKE_BINARY_DIR}/)
add_executable(Sandbox "${CMAKE_SOURCE_DIR}/src/main.cpp")
include_directories("${CMAKE_SOURCE_DIR}/src/MyEngine")
target_include_directories(Sandbox PRIVATE "${CMAKE_SOURCE_DIR}/src/MyEngine")
target_link_libraries(Sandbox PRIVATE MyEngine)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sandbox)
endif()
# -------------------------------------------
