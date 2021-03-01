cmake_minimum_required(VERSION 3.16)
project(lib_vulkan_renderer)

add_library(instance_manager INTERFACE)
target_include_directories(instance_manager
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_compile_options(instance_manager INTERFACE -Wall -Wextra -Werror)