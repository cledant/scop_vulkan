#ifndef SCOP_VULKAN_VKPHYSICALDEVICE_HPP
#define SCOP_VULKAN_VKPHYSICALDEVICE_HPP

#include <cstdint>
#include <vector>
#include <optional>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

struct DeviceRequirement
{
    std::optional<uint32_t> graphic_queue_index;
    std::optional<uint32_t> present_queue_index;
    VkBool32 geometry_shader;
    VkBool32 all_extension_supported;

    [[nodiscard]] bool isValid() const;
};

static constexpr std::array const DEVICE_EXTENSIONS{
    "VK_KHR_swapchain",
};

VkPhysicalDevice selectBestDevice(std::vector<VkPhysicalDevice> const &devices,
                                  VkSurfaceKHR surface);
int rateDevice(VkPhysicalDevice device, VkSurfaceKHR surface);
char *getDeviceName(char *dst, VkPhysicalDevice device);
DeviceRequirement getDeviceRequirement(VkPhysicalDevice device,
                                       VkSurfaceKHR surface);
void checkDeviceFeaturesSupport(VkPhysicalDevice device, DeviceRequirement &dr);
void checkDeviceExtensionSupport(VkPhysicalDevice device,
                                 DeviceRequirement &dr);
void getDeviceQueues(VkPhysicalDevice device,
                     VkSurfaceKHR surface,
                     DeviceRequirement &dr);

#endif // SCOP_VULKAN_VKPHYSICALDEVICE_HPP