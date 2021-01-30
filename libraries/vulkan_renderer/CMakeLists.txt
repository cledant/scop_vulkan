cmake_minimum_required(VERSION 3.16)
project(lib_vulkan_renderer)

add_library(vulkan_renderer STATIC
        private/VkRenderer.cpp
        private/VkPhysicalDevice.cpp
        private/VkDebug.cpp
        private/VkSwapChain.cpp
        private/VkShader.cpp
        private/VkMemory.cpp)
target_include_directories(vulkan_renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private)
set_target_properties(vulkan_renderer PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_link_libraries(vulkan_renderer
        glm)
target_compile_options(vulkan_renderer PRIVATE -Wall -Wextra -Werror)