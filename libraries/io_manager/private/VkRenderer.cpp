#include "VkRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>

void
VkRenderer::init(char const *app_name,
                 char const *engine_name,
                 uint32_t app_version,
                 uint32_t engine_version)
{
    assert(app_name);
    assert(engine_name);

    _create_instance(app_name, engine_name, app_version, engine_version);
}

void
VkRenderer::clear()
{
    vkDestroyInstance(instance, nullptr);
}

void
VkRenderer::_create_instance(char const *app_name,
                             char const *engine_name,
                             uint32_t app_version,
                             uint32_t engine_version)
{
    if (enable_validation_layer && !_check_validation_layer_support()) {
        throw std::runtime_error("VkRenderer: Validation layer not available");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = app_version;
    app_info.pEngineName = engine_name;
    app_info.engineVersion = engine_version;
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    auto extentions = _get_required_extensions();

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount =
      static_cast<uint32_t>(extentions.size());
    create_info.ppEnabledExtensionNames = extentions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    if (enable_validation_layer) {
        _setup_debug_info(debug_create_info);
        create_info.enabledLayerCount =
          static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        create_info.pNext = &debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

bool
VkRenderer::_check_validation_layer_support()
{
    uint32_t nb_avail_layers;
    vkEnumerateInstanceLayerProperties(&nb_avail_layers, nullptr);

    std::vector<VkLayerProperties> avail_layers(nb_avail_layers);
    vkEnumerateInstanceLayerProperties(&nb_avail_layers, avail_layers.data());

    for (auto const &it_val_layer : VALIDATION_LAYERS) {
        bool layer_found = false;

        for (auto const &it_avail_layer : avail_layers) {
            if (!std::strcmp(it_avail_layer.layerName, it_val_layer)) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found) {
            return (false);
        }
    }

    return (true);
}

std::vector<char const *>
VkRenderer::_get_required_extensions()
{
    uint32_t nb_glfw_extension = 0;
    const char **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&nb_glfw_extension);
    std::vector<char const *> extensions(glfw_extensions,
                                         glfw_extensions + nb_glfw_extension);

    if (enable_validation_layer) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return (extensions);
}

void
VkRenderer::_setup_debug_info(VkDebugUtilsMessengerCreateInfoEXT &create_info)
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = _debug_callback;
    create_info.pUserData = nullptr;
    create_info.pNext = nullptr;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VkRenderer::_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
  void *pUserData)
{
    static_cast<void>(messageSeverity);
    static_cast<void>(messageType);
    static_cast<void>(pUserData);
    std::cerr << "VULKAN DEBUG: " << pCallbackData->pMessage << std::endl;

    return (VK_FALSE);
}
