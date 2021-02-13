#include "VulkanDebug.hpp"

#include <iostream>

void
setupVkDebugInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info)
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
    //  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = VkDebugMsgCallback;
    create_info.pUserData = nullptr;
    create_info.pNext = nullptr;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VkDebugMsgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                   VkDebugUtilsMessageTypeFlagsEXT messageType,
                   VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                   void *pUserData)
{
    static_cast<void>(messageSeverity);
    static_cast<void>(messageType);
    static_cast<void>(pUserData);
    std::cerr << "VK DBG | " << getDbgSeverityStr(messageSeverity) << " | "
              << getDbgMessageTypeStr(messageType) << " | "
              << pCallbackData->pMessage << std::endl;

    return (VK_FALSE);
}

VkResult
createDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerCreateInfoEXT const *pCreateInfo,
  VkAllocationCallbacks const *pAllocator,
  VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return (VK_ERROR_EXTENSION_NOT_PRESENT);
    }
}

void
destroyDebugUtilsMessengerEXT(VkInstance instance,
                              VkDebugUtilsMessengerEXT debugMessenger,
                              VkAllocationCallbacks const *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

char const *
getDbgSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity)
{
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return ("Verbose");
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return ("Info");
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return ("Warning");
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return ("Error");
        default:
            return ("Unknown severity");
    }
}

char const *
getDbgMessageTypeStr(VkDebugUtilsMessageTypeFlagsEXT type)
{
    switch (type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return ("General");
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return ("Validation");
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return ("Performance");
        default:
            return ("Unknown type");
    }
}
