#ifndef SCOP_VULKAN_VKPHYSICALDEVICE_HPP
#define SCOP_VULKAN_VKPHYSICALDEVICE_HPP

#include <cstdint>
#include <vector>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

VkPhysicalDevice selectBestDevice(std::vector<VkPhysicalDevice> const &devices);
int rateDevice(VkPhysicalDevice device);
char *getDeviceName(char *dst, VkPhysicalDevice device);
bool hasDeviceGeometryShader(VkPhysicalDevice device);
bool hasDeviceGraphicQueue(VkPhysicalDevice device);
std::optional<uint32_t> getGraphicQueueIndex(VkPhysicalDevice device);

#endif // SCOP_VULKAN_VKPHYSICALDEVICE_HPP
