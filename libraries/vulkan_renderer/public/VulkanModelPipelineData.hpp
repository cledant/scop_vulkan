#ifndef SCOP_VULKAN_VULKANMODELPIPELINEDATA_HPP
#define SCOP_VULKAN_VULKANMODELPIPELINEDATA_HPP

#include <array>
#include <vector>

#include <vulkan/vulkan.h>

#include "glm/glm.hpp"

#include "VulkanTextureManager.hpp"

struct VulkanModelPipelineData
{
    VkBuffer buffer{};
    VkDeviceMemory memory{};
    Texture diffuseTexture{};
    VkDeviceSize verticesSize{};
    VkDeviceSize indicesSize{};
    VkDeviceSize nbIndices{};
    VkDeviceSize singleUboSize{};
    VkDeviceSize instanceMatricesOffset{};
    VkDeviceSize indicesOffset{};
    VkDeviceSize uboOffset{};
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets;
    glm::vec3 meshCenter{};

    static std::array<VkVertexInputBindingDescription, 2>
    getInputBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 9>
    getInputAttributeDescription();
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINEDATA_HPP
