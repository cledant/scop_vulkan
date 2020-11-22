#include "VkPhysicalDevice.hpp"

#include <map>
#include <cstring>
#include <cassert>

VkPhysicalDevice
selectBestDevice(std::vector<VkPhysicalDevice> const &devices,
                 VkSurfaceKHR surface)
{
    auto best_device = std::make_pair<VkPhysicalDevice, int>(VK_NULL_HANDLE, 0);
    std::multimap<VkPhysicalDevice, int> rating;

    for (auto const &it : devices) {
        auto score = rateDevice(it, surface);
        rating.insert(std::make_pair(it, score));
    }

    for (auto const &it : rating) {
        if (it.second > best_device.second) {
            best_device = it;
        }
    }
    return (best_device.first);
}

int
rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    auto dfr = getDeviceFeatureRequirement(device, surface);

    if (!dfr.isValid()) {
        return (0);
    }

    int score = 0;
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    switch (properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 1000000;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 5000;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            score += 1;
            break;
        default:
            score += 1000;
            break;
    }

    VkPhysicalDeviceMemoryProperties mem;
    vkGetPhysicalDeviceMemoryProperties(device, &mem);
    std::optional<uint32_t> index_local_memory;
    for (uint32_t i = 0; i < mem.memoryTypeCount; ++i) {
        if (mem.memoryTypes[i].propertyFlags &
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            index_local_memory = mem.memoryTypes[i].heapIndex;
            break;
        }
    }
    if (!index_local_memory.has_value()) {
        return (0);
    }
    score += mem.memoryHeaps[index_local_memory.value()].size / (1024 * 1024);
    return (score);
}

char *
getDeviceName(char *dst, VkPhysicalDevice device)
{
    assert(dst);

    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(device, &prop);
    std::strncpy(dst, prop.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
    return (dst);
}

DeviceFeatureRequirement
getDeviceFeatureRequirement(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    DeviceFeatureRequirement dfr{};
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    if (features.geometryShader) {
        dfr.geometry_shader = VK_TRUE;
    }

    uint32_t nb_family_queue;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &nb_family_queue, nullptr);
    std::vector<VkQueueFamilyProperties> families(nb_family_queue);
    vkGetPhysicalDeviceQueueFamilyProperties(
      device, &nb_family_queue, families.data());

    uint32_t index = 0;
    for (auto const &it : families) {
        if (it.queueFlags & VK_QUEUE_GRAPHICS_BIT && it.queueCount > 0) {
            dfr.graphic_queue_index = index;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(
          device, index, surface, &present_support);
        if (present_support) {
            dfr.present_queue_index = index;
        }

        if (dfr.graphic_queue_index.has_value() &&
            dfr.present_queue_index.has_value()) {
            break;
        }
        ++index;
    }
    return (dfr);
}

bool
DeviceFeatureRequirement::isValid() const
{
    return (graphic_queue_index.has_value() &&
            present_queue_index.has_value() && geometry_shader);
}
