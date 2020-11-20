#include "VkPhysicalDevice.hpp"

#include <map>
#include <optional>
#include <cstring>
#include <cassert>
#include <iostream>

VkPhysicalDevice
selectBestDevice(std::vector<VkPhysicalDevice> const &devices)
{
    auto best_device = std::make_pair<VkPhysicalDevice, int>(VK_NULL_HANDLE, 0);
    std::multimap<VkPhysicalDevice, int> rating;

    for (auto const &it : devices) {
        auto score = rateDevice(it);
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
rateDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    VkPhysicalDeviceMemoryProperties mem;
    vkGetPhysicalDeviceMemoryProperties(device, &mem);

    int score = 0;

    if (!features.geometryShader) {
        return (score);
    }

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
