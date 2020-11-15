cmake_minimum_required(VERSION 3.10)
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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glm ${CMAKE_BINARY_DIR}/external/glm/build)

#Setup GLFW
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_INSTALL "" OFF)
option(BUILD_SHARED_LIBS "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw ${CMAKE_BINARY_DIR}/external/glfw/build)

#Projet libs
add_subdirectory(libraries/io_manager)
add_subdirectory(libraries/engine)

#Main binary
add_executable(scop
        ${CMAKE_CURRENT_SOURCE_DIR}/binary/private/scop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/binary/private/ArgsParsing.cpp)
target_include_directories(scop
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/binary/private)
target_link_libraries(scop iomanager engine)
add_dependencies(scop iomanager engine)
set_target_properties(scop PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_compile_options(scop PRIVATE -Wall -Wextra -Werror)