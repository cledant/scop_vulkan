#include "VkRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>

#include "VkDebug.hpp"
#include "VkPhysicalDevice.hpp"

void
VkRenderer::init(char const *app_name,
                 char const *engine_name,
                 uint32_t app_version,
                 uint32_t engine_version)
{
    assert(app_name);
    assert(engine_name);

    _create_instance(app_name, engine_name, app_version, engine_version);
    _setup_vk_debug_msg();
    _select_physical_device();
}

void
VkRenderer::clear()
{
    if constexpr (ENABLE_VALIDATION_LAYER) {
        destroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
    }
    vkDestroyInstance(_instance, nullptr);
}

void
VkRenderer::_create_instance(char const *app_name,
                             char const *engine_name,
                             uint32_t app_version,
                             uint32_t engine_version)
{
    if (ENABLE_VALIDATION_LAYER && !_check_validation_layer_support()) {
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
    if constexpr (ENABLE_VALIDATION_LAYER) {
        setupVkDebugInfo(debug_create_info);
        create_info.enabledLayerCount =
          static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
        create_info.pNext = &debug_create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }

    if (vkCreateInstance(&create_info, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create instance");
    }
}

void
VkRenderer::_setup_vk_debug_msg()
{
    if constexpr (!ENABLE_VALIDATION_LAYER) {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT dbg_info{};
    setupVkDebugInfo(dbg_info);

    createDebugUtilsMessengerEXT(
      _instance, &dbg_info, nullptr, &_debug_messenger);
}

void
VkRenderer::_select_physical_device()
{
    uint32_t nb_physical_device = 0;
    vkEnumeratePhysicalDevices(_instance, &nb_physical_device, nullptr);
    if (!nb_physical_device) {
        throw std::runtime_error("VkRenderer: No physical device");
    }

    std::vector<VkPhysicalDevice> devices(nb_physical_device);
    vkEnumeratePhysicalDevices(_instance, &nb_physical_device, devices.data());

    _physical_device = selectBestDevice(devices);
    if (_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("VkRenderer: No Suitable device found");
    }
    getDeviceName(_device_name, _physical_device);
    std::cout << "Device: " << _device_name << std::endl;
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
    char const **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&nb_glfw_extension);
    std::vector<char const *> extensions(glfw_extensions,
                                         glfw_extensions + nb_glfw_extension);

    if constexpr (ENABLE_VALIDATION_LAYER) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return (extensions);
}
