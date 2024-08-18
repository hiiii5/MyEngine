function(FIND_FMT)
    include(FetchContent)

    FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         e69e5f977d458f2650bb346dadf2ad30c5320281 # 10.2.1
        GIT_SHALLOW     TRUE
        GIT_PROGRESS    TRUE
    )
    FetchContent_MakeAvailable(fmt)
endfunction()
