function(COMPILE_SHADERS)
    file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/shaders/*.glsl"
    )

    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(GLSL_FILENAME ${GLSL} NAME_WLE)
        message("GLSL Filename: ${GLSL_FILENAME}")
        set(SPIRV_FILENAME "${CMAKE_SOURCE_DIR}/shaders/${GLSL_FILENAME}.spv")
        add_custom_command(
            OUTPUT ${SPIRV_FILENAME}
            COMMAND "$ENV{GLSL_VALIDATOR}" -V ${GLSL} -o ${SPIRV_FILENAME}
            DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV_FILENAME})
    endforeach(GLSL)

    add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
    )
endfunction()
