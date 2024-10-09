cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

project(MyEngine)

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

if(LINUX)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
# -------------------------------------------

# -------------------------------------------
# BUNDLE PACKAGES BY PLATFORM
# -------------------------------------------
if(LINUX)
  find_package(Threads REQUIRED)
endif()

include("${CMAKE_SOURCE_DIR}/cmake/find_glm.cmake")
find_glm()

include("${CMAKE_SOURCE_DIR}/cmake/find_vulkan.cmake")
find_vulkan()

include("${CMAKE_SOURCE_DIR}/cmake/find_sdl.cmake")
find_sdl()

include("${CMAKE_SOURCE_DIR}/cmake/find_spdlog.cmake")
find_spdlog()

add_subdirectory("${CMAKE_SOURCE_DIR}/MyEngine/vendor/shaderc")

# IMGUI is special
set(IMGUI_PATH "${CMAKE_SOURCE_DIR}/MyEngine/vendor/imgui")
file(
  GLOB
  IMGUI_SOURCES
  "${IMGUI_PATH}/imgui.cpp"
  "${IMGUI_PATH}/imgui_demo.cpp"
  "${IMGUI_PATH}/imgui_draw.cpp"
  "${IMGUI_PATH}/imgui_tables.cpp"
  "${IMGUI_PATH}/imgui_widgets.cpp"
  # backend bindings
  "${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp"
  "${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp")
add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC ${IMGUI_PATH} "${IMGUI_PATH}/backends")
target_link_libraries(ImGui PRIVATE SDL2::SDL2 Vulkan::Vulkan)
# -------------------------------------------

# -------------------------------------------
# GLSL TO SPIRV
# -------------------------------------------
# include("${CMAKE_SOURCE_DIR}/cmake/compile_shaders.cmake") compile_shaders()
# -------------------------------------------

# -------------------------------------------
# COMPILE/LINKING
# -------------------------------------------
file(GLOB_RECURSE SOURCE "${CMAKE_SOURCE_DIR}/MyEngine/src/*.cpp")

if(WIN32)

elseif(APPLE)
  link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
else(WIN32)
  link_libraries(${Vulkan_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
endif(WIN32)

add_library(MyEngine STATIC ${SOURCE})

target_compile_definitions(MyEngine PUBLIC $<$<CONFIG:Debug>:ME_DEBUG>)

include_directories("${CMAKE_SOURCE_DIR}/MyEngine/src")
target_precompile_headers(MyEngine PRIVATE
                          "${CMAKE_SOURCE_DIR}/MyEngine/src/mepch.h")

# add_dependencies(MyEngine Shaders)

if(LINUX)
  target_link_libraries(MyEngine PRIVATE ImGui SDL2::SDL2 spdlog::spdlog
                                         Threads::Threads shaderc_combined)
else()
  target_link_libraries(MyEngine PRIVATE ImGui SDL2::SDL2 spdlog::spdlog)
endif()

# -------------------------------------------
