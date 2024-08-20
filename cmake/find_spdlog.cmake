function(FIND_SPDLOG)
    include(FetchContent)

    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY  https://github.com/gabime/spdlog.git
        GIT_TAG         v1.x
        GIT_SHALLOW     TRUE
        GIT_PROGRESS    TRUE
    )
    FetchContent_MakeAvailable(spdlog)
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/_deps/spdlog-src/include")
endfunction()
