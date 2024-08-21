function(FIND_VULKAN)
  find_package(Vulkan)

  if(NOT Vulkan_FOUND)
    message(
      STATUS "Vulkan package was not found locally, attempting to fetch content"
    )
    include(FetchContent)

    set(VULKAN_VER_MAJOR 1)
    set(VULKAN_VER_MINOR 3)
    set(VULKAN_VER_PATCH 278)
    set(VULKAN_VERSION
        ${VULKAN_VER_MAJOR}.${VULKAN_VER_MINOR}.${VULKAN_VER_PATCH})

    # -------------------------------------------
    # FETCH VULKAN C HEADERS
    # -------------------------------------------
    message(STATUS "Including Vulkan C Headers")
    FetchContent_Declare(
      vulkanheaders
      GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
      GIT_TAG v${VULKAN_VERSION}
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE)
    set(PROJECT_IS_TOP_LEVEL OFF)
    # vulkanheaders has a proper CMakeLists
    FetchContent_MakeAvailable(vulkanheaders)

    # -------------------------------------------
    # FETCH VULKAN C++ HEADERS
    # -------------------------------------------
    message(STATUS "Including Vulkan C++ Headers")
    FetchContent_Declare(
      vulkanhpp
      GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Hpp.git
      GIT_TAG v${VULKAN_VERSION}
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE)
    # vulkanhpp does not have a proper CMakeLists
    FetchContent_GetProperties(vulkanhpp)
    FetchContent_Populate(vulkanhpp)
  endif()

  # Define a library for Vulkan
  add_library(vulkan INTERFACE)
  target_include_directories(vulkan INTERFACE ${vulkanhpp_SOURCE_DIR})
  target_link_libraries(vulkan INTERFACE Vulkan::Headers)
endfunction()
