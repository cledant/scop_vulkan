cmake_minimum_required(VERSION 3.17)
project(scop_model_shader)

set(SHADER_NAME model)
set(SHADER_SOURCE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR})
set(SHADER_RUNTIME_FOLDER
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/shaders/${SHADER_NAME})

#Filelist
set(SHADERS
        ${SHADER_SOURCE_FOLDER}/${SHADER_NAME}.frag
        ${SHADER_SOURCE_FOLDER}/${SHADER_NAME}.vert)
set(COMPILED_SHADERS
        ${SHADER_RUNTIME_FOLDER}/${SHADER_NAME}.frag.spv
        ${SHADER_RUNTIME_FOLDER}/${SHADER_NAME}.vert.spv)

file(MAKE_DIRECTORY ${SHADER_RUNTIME_FOLDER})
foreach (SHADER COMPILED_SHADER IN ZIP_LISTS SHADERS COMPILED_SHADERS)
    add_custom_command(
            OUTPUT ${COMPILED_SHADER}
            DEPENDS ${SHADER}
            COMMAND
            "${GLSLC_PROGRAM}"
            -o ${COMPILED_SHADER} -mfmt=bin -O
            --target-env=vulkan1.2
            ${SHADER}
            -Werror
            COMMENT "Building: ${SHADER}"
            VERBATIM
    )
endforeach ()

add_custom_target(model_shader DEPENDS ${COMPILED_SHADERS})
