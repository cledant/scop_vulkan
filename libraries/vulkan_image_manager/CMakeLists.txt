cmake_minimum_required(VERSION 3.16)
project(lib_vulkan_image_manager)

add_library(vulkan_image_manager STATIC
        private/VkImageManager.cpp)
target_include_directories(vulkan_image_manager
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public)
set_target_properties(vulkan_image_manager PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
add_dependencies(vulkan_image_manager
        vulkan_utils)
target_link_libraries(vulkan_image_manager
        vulkan_utils
        glm)
target_compile_options(vulkan_image_manager PRIVATE -Wall -Wextra -Werror)