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
    VkDeviceSize verticesSize{};
    VkDeviceSize indicesSize{};
    VkDeviceSize singleUboSize{};
    VkDeviceSize singleSwapChainUboSize{};
    VkDeviceSize instanceMatricesOffset{};
    VkDeviceSize indicesOffset{};
    VkDeviceSize uboOffset{};
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets;
    glm::vec3 modelCenter{};
    VkDeviceSize nbMaterials{};
    std::vector<Texture> diffuseTextures;
    std::vector<VkDeviceSize> indicesDrawOffset;
    std::vector<VkDeviceSize> indicesDrawNb;

    static std::array<VkVertexInputBindingDescription, 2>
    getInputBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 9>
    getInputAttributeDescription();

    void clear();
};

#endif // SCOP_VULKAN_VULKANMODELPIPELINEDATA_HPP
