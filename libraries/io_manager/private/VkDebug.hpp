#ifndef SCOP_VULKAN_VKDEBUG_HPP
#define SCOP_VULKAN_VKDEBUG_HPP

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

void setupVkDebugInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);
VKAPI_ATTR VkBool32 VKAPI_CALL
VkDebugMsgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                   VkDebugUtilsMessageTypeFlagsEXT messageType,
                   VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                   void *pUserData);
VkResult createDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerCreateInfoEXT const *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkDebugUtilsMessengerEXT *pDebugMessenger);
void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   VkAllocationCallbacks const *pAllocator);

#endif // SCOP_VULKAN_VKDEBUG_HPP
