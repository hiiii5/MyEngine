function(FIND_SDL)
  find_package(SDL2 CONFIG REQUIRED COMPONENTS SDL2)

  if(NOT SDL2_FOUND)
    include(FetchContent)

    FetchContent_Declare(
      SDL2
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG release-2.30.6
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE)
    FetchContent_MakeAvailable(SDL2)
  endif()
endfunction()
