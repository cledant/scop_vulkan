cmake_minimum_required(VERSION 3.17)
project(scop_vulkan)

#Detecting + setting global LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_output)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND lto_supported)
    message("Compiling with LTO enabled")
    cmake_policy(SET CMP0069 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else ()
    message("LTO not supported")
endif ()

#Runtime resources
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Checking Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
find_program(GLSLC_PROGRAM glslc REQUIRED FATAL_ERROR)

#Setting global C/CXX compile options
set(OPTIMIZATION_TYPE NATIVE CACHE STRING "")

if (${OPTIMIZATION_TYPE} STREQUAL NATIVE)
    message("Build binary for current computer arch")
    set(CMAKE_CXX_FLAGS "-march=native -pipe")
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
elseif (${OPTIMIZATION_TYPE} STREQUAL AVX2)
    message("Build binary with AVX2 profile")
    set(CMAKE_CXX_FLAGS "-march=core2 -mtune=haswell \
    -mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4.1 -msse4.2 \
    -mavx -mavx2 -mbmi -mbmi2 -mf16c -mfma -mfsgsbase \
    -mlzcnt -mmovbe -mpclmul -mpopcnt -maes \
    -mrdrnd  -pipe")
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
elseif (${OPTIMIZATION_TYPE} STREQUAL SSE4)
    message("Build binary with SSE4 profile")
    set(CMAKE_CXX_FLAGS "-march=core2 -mtune=nehalem \
    -mmmx -msse -msse2 -msse3 -mssse3 -msse4 -msse4.1 -msse4.2 \
    -mpopcnt -pipe")
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
else ()
    message("No arch optimization selected")
    set(CMAKE_CXX_FLAGS "-pipe")
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
endif ()

#Setup glm
add_library(glm INTERFACE IMPORTED)
set_target_properties(glm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)

#Setup GLFW
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_INSTALL "" OFF)
option(BUILD_SHARED_LIBS "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw ${CMAKE_BINARY_DIR}/external/glfw/build)

#Setup stb
add_library(stb INTERFACE IMPORTED)
set_target_properties(stb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)

#Setup Assimp
option(BUILD_SHARED_LIBS "" OFF)
option(ASSIMP_NO_EXPORT "" ON)
option(ASSIMP_BUILD_TESTS "" OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS "" ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/assimp ${CMAKE_BINARY_DIR}/external/assimp/build)

#Setup fmt
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/fmt ${CMAKE_BINARY_DIR}/external/fmt/build)

#Setup imgui
set(IMGUI_FOLDER ${CMAKE_SOURCE_DIR}/external/imgui)
add_library(imgui_glfw_vulkan STATIC
        ${IMGUI_FOLDER}/backends/imgui_impl_glfw.cpp
        ${IMGUI_FOLDER}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_FOLDER}/imgui.cpp
        ${IMGUI_FOLDER}/imgui_draw.cpp
        ${IMGUI_FOLDER}/imgui_demo.cpp
        ${IMGUI_FOLDER}/imgui_tables.cpp
        ${IMGUI_FOLDER}/imgui_widgets.cpp)
target_include_directories(imgui_glfw_vulkan
        PUBLIC
        ${IMGUI_FOLDER}
        ${IMGUI_FOLDER}/backend)
add_dependencies(imgui_glfw_vulkan
        glfw)
target_link_libraries(imgui_glfw_vulkan
        glfw)

#Project libs
add_subdirectory(libraries/io_manager)
add_subdirectory(libraries/engine)
add_subdirectory(libraries/vulkan_utils)
add_subdirectory(libraries/vulkan_renderer)
add_subdirectory(libraries/model)
add_subdirectory(libraries/indexed_buffer)
add_subdirectory(libraries/ui)
add_subdirectory(libraries/app_version)

#Shaders
add_subdirectory(shaders/model)

#Main binary
add_executable(scop
        ${CMAKE_CURRENT_SOURCE_DIR}/binary/private/scop.cpp)
target_include_directories(scop
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/binary/private)
target_link_libraries(scop iomanager engine vulkan)
add_dependencies(scop iomanager engine model_shader ui)
set_target_properties(scop PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_compile_options(scop PRIVATE -Wall -Wextra -Werror)
