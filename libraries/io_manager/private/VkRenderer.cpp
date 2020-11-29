#include "VkRenderer.hpp"

#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>
#include <iostream>
#include <set>

#include "VkDebug.hpp"
#include "VkPhysicalDevice.hpp"

void
VkRenderer::init(char const *app_name,
                 char const *engine_name,
                 GLFWwindow *win,
                 uint32_t app_version,
                 uint32_t engine_version)
{
    assert(app_name);
    assert(engine_name);
    assert(win);

    _create_instance(app_name, engine_name, app_version, engine_version);
    if (glfwCreateWindowSurface(_instance, win, nullptr, &_surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create window surface");
    }
    _setup_vk_debug_msg();
    _select_physical_device();
    _create_graphic_queue();
    _create_swap_chain();
}

void
VkRenderer::clear()
{
    //vkDestroySwapchainKHR(_device, _swap_chain, nullptr);
    vkDestroyDevice(_device, nullptr);
    if constexpr (ENABLE_VALIDATION_LAYER) {
        destroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
    }
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
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

    _physical_device = selectBestDevice(devices, _surface);
    if (_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("VkRenderer: No Suitable device found");
    }
    getDeviceName(_device_name, _physical_device);
    std::cout << "Device: " << _device_name << std::endl;
}

void
VkRenderer::_create_graphic_queue()
{
    auto dfr = getDeviceRequirement(_physical_device, _surface);
    std::set<uint32_t> queue_families = { dfr.graphic_queue_index.value(),
                                          dfr.present_queue_index.value() };
    std::vector<VkDeviceQueueCreateInfo> vec_queue_create_info;

    // Graphic queue info
    float queue_priority = 1.0f;
    for (auto const &it : queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = it;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        vec_queue_create_info.emplace_back(queue_create_info);
    }

    // Device info
    VkPhysicalDeviceFeatures physical_device_features{};
    physical_device_features.geometryShader = VK_TRUE;
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = vec_queue_create_info.data();
    device_create_info.queueCreateInfoCount = vec_queue_create_info.size();
    if constexpr (ENABLE_VALIDATION_LAYER) {
        device_create_info.enabledLayerCount =
          static_cast<uint32_t>(VALIDATION_LAYERS.size());
        device_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        device_create_info.enabledLayerCount = 0;
    }
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.enabledExtensionCount = 0;

    // Device creation
    if (vkCreateDevice(
          _physical_device, &device_create_info, nullptr, &_device) !=
        VK_SUCCESS) {
        throw std::runtime_error("VkRenderer: Failed to create logical device");
    }
    vkGetDeviceQueue(
      _device, dfr.graphic_queue_index.value(), 0, &_graphic_queue);
    vkGetDeviceQueue(
      _device, dfr.present_queue_index.value(), 0, &_present_queue);
}

void
VkRenderer::_create_swap_chain()
{}

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
