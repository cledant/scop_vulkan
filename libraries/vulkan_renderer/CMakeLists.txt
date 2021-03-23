cmake_minimum_required(VERSION 3.17)
project(lib_vulkan_renderer)

add_library(vulkan_renderer STATIC
        private/VulkanRenderer.cpp
        private/VulkanDebug.cpp
        private/VulkanInstance.cpp
        private/VulkanSwapChain.cpp
        private/VulkanSync.cpp
        private/VulkanTextureManager.cpp
        private/VulkanModelPipeline.cpp
        private/VulkanModelPipelineData.cpp
        private/VulkanModelRenderPass.cpp
        private/VulkanUiRenderPass.cpp
        private/VulkanUi.cpp)
target_include_directories(vulkan_renderer
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private)
set_target_properties(vulkan_renderer PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
add_dependencies(vulkan_renderer
        vulkan_utils
        fmt
        model
        imgui_glfw_vulkan)
target_link_libraries(vulkan_renderer
        instance_manager
        glm
        fmt
        vulkan_utils
        model
        imgui_glfw_vulkan)
target_compile_options(vulkan_renderer PRIVATE -Wall -Wextra -Werror)