cmake_minimum_required(VERSION 3.16)
project(lib_iomanager)

add_library(iomanager STATIC
        private/IOManager.cpp)
target_include_directories(iomanager
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public)
add_dependencies(iomanager glfw glm vulkan_renderer)
target_link_libraries(iomanager PUBLIC glfw glm vulkan_renderer)
set_target_properties(iomanager PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_compile_options(iomanager PRIVATE -Wall -Wextra -Werror)