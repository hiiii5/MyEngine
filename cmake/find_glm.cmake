function(FIND_GLM)
    include(FetchContent)

    FetchContent_Declare(
        glm
        GIT_REPOSITORY  http://github.com/g-truc/glm.git
        GIT_TAG         bf71a834948186f4097caa076cd2663c69a10e1e
        GIT_SHALLOW     TRUE
        GIT_PROGRESS    TRUE
    )
    FetchContent_MakeAvailable(glm)

    add_library(glmath INTERFACE)
    target_link_libraries(glmath INTERFACE glm::glm)
endfunction()
