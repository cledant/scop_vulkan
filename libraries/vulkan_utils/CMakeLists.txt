cmake_minimum_required(VERSION 3.17)
project(lib_vulkan_utils)

add_library(vulkan_utils STATIC
        private/VulkanPhysicalDevice.cpp
        private/VulkanSwapChainUtils.cpp
        private/VulkanShader.cpp
        private/VulkanImage.cpp
        private/VulkanMemory.cpp
        private/VulkanCommandBuffer.cpp)
target_include_directories(vulkan_utils
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public)
set_target_properties(vulkan_utils PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_link_libraries(vulkan_utils
        glm
        stb)
target_compile_options(vulkan_utils PRIVATE -Wall -Wextra -Werror)