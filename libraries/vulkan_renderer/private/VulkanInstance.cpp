#include "VulkanInstance.hpp"

#include <cstring>
#include <stdexcept>
#include <set>

#include "fmt/core.h"

#include "VulkanDebug.hpp"
#include "VulkanPhysicalDevice.hpp"

VkInstance
VulkanInstance::createInstance(std::string const &app_name,
                               std::string const &engine_name,
                               uint32_t app_version,
                               uint32_t engine_version,
                               std::vector<char const *> &&required_extensions)
{
    if (ENABLE_VALIDATION_LAYER && !_check_validation_layer_support()) {
        throw std::runtime_error(
          "VulkanInstance: Validation layer not available");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name.c_str();
    app_info.applicationVersion = app_version;
    app_info.pEngineName = engine_name.c_str();
    app_info.engineVersion = engine_version;
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount =
      static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();

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

    VkInstance instance;
    if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("VulkanInstance: Failed to create instance");
    }
    return (instance);
}

void
VulkanInstance::init(VkSurfaceKHR windowSurface)
{
    surface = windowSurface;
    _setup_vk_debug_msg();
    _select_physical_device();
    _create_present_and_graphic_queue();
    _create_command_pool();
}

void
VulkanInstance::clear()
{
    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyDevice(device, nullptr);
    if constexpr (ENABLE_VALIDATION_LAYER) {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    instance = nullptr;
    surface = nullptr;
    debugMessenger = nullptr;
    physicalDevice = nullptr;
    device = nullptr;
    graphicQueue = nullptr;
    presentQueue = nullptr;
    commandPool = nullptr;
}

void
VulkanInstance::_setup_vk_debug_msg()
{
    if constexpr (!ENABLE_VALIDATION_LAYER) {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT dbg_info{};
    setupVkDebugInfo(dbg_info);

    createDebugUtilsMessengerEXT(instance, &dbg_info, nullptr, &debugMessenger);
}

void
VulkanInstance::_select_physical_device()
{
    uint32_t nb_physical_device = 0;
    vkEnumeratePhysicalDevices(instance, &nb_physical_device, nullptr);
    if (!nb_physical_device) {
        throw std::runtime_error("VulkanInstance: No physical device");
    }

    std::vector<VkPhysicalDevice> devices(nb_physical_device);
    vkEnumeratePhysicalDevices(instance, &nb_physical_device, devices.data());

    physicalDevice = selectBestDevice(devices, surface);
    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("VulkanInstance: No Suitable device found");
    }
    getDeviceName(deviceName, physicalDevice);
    fmt::print("Device: {}\n", deviceName);
}

void
VulkanInstance::_create_present_and_graphic_queue()
{
    auto dfr = getDeviceRequirement(physicalDevice, surface);
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
    physical_device_features.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = vec_queue_create_info.data();
    device_create_info.queueCreateInfoCount = vec_queue_create_info.size();
    device_create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    device_create_info.enabledExtensionCount = DEVICE_EXTENSIONS.size();
    if constexpr (ENABLE_VALIDATION_LAYER) {
        device_create_info.enabledLayerCount =
          static_cast<uint32_t>(VALIDATION_LAYERS.size());
        device_create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        device_create_info.enabledLayerCount = 0;
    }
    device_create_info.pEnabledFeatures = &physical_device_features;

    // Device creation
    if (vkCreateDevice(physicalDevice, &device_create_info, nullptr, &device) !=
        VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanInstance: Failed to create logical device");
    }
    vkGetDeviceQueue(device, dfr.graphic_queue_index.value(), 0, &graphicQueue);
    vkGetDeviceQueue(device, dfr.present_queue_index.value(), 0, &presentQueue);
    graphicQueueIndex = dfr.graphic_queue_index.value();
    presentQueueIndex = dfr.present_queue_index.value();
}

void
VulkanInstance::_create_command_pool()
{
    DeviceRequirement dr{};
    getDeviceQueues(physicalDevice, surface, dr);

    VkCommandPoolCreateInfo command_pool_info{};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.queueFamilyIndex = dr.graphic_queue_index.value();
    command_pool_info.flags = 0;

    if (vkCreateCommandPool(
          device, &command_pool_info, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error(
          "VulkanInstance: Failed to create command pool");
    }
}

// Dbg related
bool
VulkanInstance::_check_validation_layer_support()
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