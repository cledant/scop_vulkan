#ifndef SCOP_VULKAN_VKPHYSICALDEVICE_HPP
#define SCOP_VULKAN_VKPHYSICALDEVICE_HPP

#include <cstdint>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

VkPhysicalDevice selectBestDevice(std::vector<VkPhysicalDevice> const &devices);
int rateDevice(VkPhysicalDevice device);
char *getDeviceName(char *dst, VkPhysicalDevice device);

#endif // SCOP_VULKAN_VKPHYSICALDEVICE_HPP
