#include "VulkanSwapChainUtils.hpp"

SwapChainSupport
getSwapChainSupport(VkPhysicalDevice device,
                    VkSurfaceKHR surface,
                    VkExtent2D actual_extent)
{
    SwapChainSupport scs{};

    auto scs_info = getSwapChainSupportInfo(device, surface);
    scs.capabilities = scs_info.capabilities;
    scs.surface_format = getSwapChainSurfaceFormat(scs_info.formats);
    scs.present_mode = getSwapChainPresentMode(scs_info.present_mode);
    scs.extent = getSwapChainExtent(scs_info.capabilities, actual_extent);
    return (scs);
}

bool
checkSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupport scs{};

    auto scs_info = getSwapChainSupportInfo(device, surface);
    scs.surface_format = getSwapChainSurfaceFormat(scs_info.formats);
    scs.present_mode = getSwapChainPresentMode(scs_info.present_mode);
    return (scs.isValid());
}

SwapChainSupportInfo
getSwapChainSupportInfo(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportInfo scsi{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &scsi.capabilities);

    uint32_t nb_format;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &nb_format, nullptr);
    if (nb_format) {
        scsi.formats.resize(nb_format);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
          device, surface, &nb_format, scsi.formats.data());
    }

    uint32_t nb_present_mode;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &nb_present_mode, nullptr);
    if (nb_present_mode) {
        scsi.present_mode.resize(nb_present_mode);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
          device, surface, &nb_present_mode, scsi.present_mode.data());
    }
    return (scsi);
}

std::optional<VkSurfaceFormatKHR>
getSwapChainSurfaceFormat(
  std::vector<VkSurfaceFormatKHR> const &available_formats)
{
    std::optional<VkSurfaceFormatKHR> surface_format{};

    for (auto const &it : available_formats) {
        if (it.format == VK_FORMAT_B8G8R8A8_SRGB &&
            it.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            surface_format = it;
            break;
        }
    }
    return (surface_format);
}

std::optional<VkPresentModeKHR>
getSwapChainPresentMode(
  std::vector<VkPresentModeKHR> const &available_present_mode)
{
    std::optional<VkPresentModeKHR> present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (auto const &it : available_present_mode) {
        if (it == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            present_mode = it;
            break;
        }
    }
    return (present_mode);
}

VkExtent2D
getSwapChainExtent(VkSurfaceCapabilitiesKHR const &capabilities,
                   VkExtent2D actual_extent)
{
    if (capabilities.currentExtent.height != UINT32_MAX) {
        return (capabilities.currentExtent);
    }

    VkExtent2D real_extent;
    real_extent.width = std::max(
      capabilities.minImageExtent.width,
      std::min(capabilities.maxImageExtent.width, actual_extent.width));
    real_extent.height = std::max(
      capabilities.minImageExtent.height,
      std::min(capabilities.maxImageExtent.height, actual_extent.height));
    return (real_extent);
}

bool
SwapChainSupport::isValid() const
{
    return (surface_format.has_value() && present_mode.has_value());
}
