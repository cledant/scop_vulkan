cmake_minimum_required(VERSION 3.17)
project(lib_ui)

add_library(ui STATIC
        private/Ui.cpp
        private/UiOpenModel.cpp
        private/UiInfoOverview.cpp)
target_include_directories(ui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public)
add_dependencies(ui
        glfw
        imgui_glfw_vulkan
        vulkan_utils
        vulkan_renderer)
target_link_libraries(ui PUBLIC
        vulkan
        glfw
        imgui_glfw_vulkan
        vulkan_renderer
        vulkan_utils)
set_target_properties(ui PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO)
target_compile_options(ui PRIVATE -Wall -Wextra -Werror)