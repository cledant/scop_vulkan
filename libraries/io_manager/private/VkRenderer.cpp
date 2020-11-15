#include "VkRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>

void
VkRenderer::init(char const *app_name,
                 char const *engine_name,
                 uint32_t app_version,
                 uint32_t engine_version)
{
    assert(app_name);
    assert(engine_name);

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = app_version;
    app_info.pEngineName = engine_name;
    app_info.engineVersion = engine_version;
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    uint32_t nb_glfw_extension = 0;
    const char **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&nb_glfw_extension);

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = nb_glfw_extension;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    create_info.enabledLayerCount = 0;

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void
VkRenderer::clear()
{
    vkDestroyInstance(instance, nullptr);
}