#ifndef SCOP_VULKAN_VKPHYSICALDEVICE_HPP
#define SCOP_VULKAN_VKPHYSICALDEVICE_HPP

#include <cstdint>
#include <vector>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

struct DeviceFeatureRequirement
{
    std::optional<uint32_t> graphic_queue_index;
    std::optional<uint32_t> present_queue_index;
    VkBool32 geometry_shader;

    [[nodiscard]] bool isValid() const;
};

VkPhysicalDevice selectBestDevice(std::vector<VkPhysicalDevice> const &devices,
                                  VkSurfaceKHR surface);
int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
char *getDeviceName(char *dst, VkPhysicalDevice device);
DeviceFeatureRequirement getDeviceFeatureRequirement(VkPhysicalDevice device,
                                                     VkSurfaceKHR surface);

#endif // SCOP_VULKAN_VKPHYSICALDEVICE_HPP
