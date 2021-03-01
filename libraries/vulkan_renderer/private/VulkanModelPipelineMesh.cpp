#include "VulkanModelPipelineMesh.hpp"

#include "Mesh.hpp"

std::array<VkVertexInputBindingDescription, 2>
VulkanModelPipelineMesh::getInputBindingDescription()
{
    std::array<VkVertexInputBindingDescription, 2> binding_description{};
    binding_description[0].binding = 0;
    binding_description[0].stride = sizeof(Vertex);
    binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    binding_description[1].binding = 1;
    binding_description[1].stride = sizeof(glm::mat4);
    binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return (binding_description);
}

std::array<VkVertexInputAttributeDescription, 9>
VulkanModelPipelineMesh::getInputAttributeDescription()
{
    std::array<VkVertexInputAttributeDescription, 9> attribute_description{};

    attribute_description[0].binding = 0;
    attribute_description[0].location = 0;
    attribute_description[0].offset = 0;
    attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[1].binding = 0;
    attribute_description[1].location = 1;
    attribute_description[1].offset = offsetof(Vertex, normal);
    attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[2].binding = 0;
    attribute_description[2].location = 2;
    attribute_description[2].offset = offsetof(Vertex, tex_coords);
    attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;

    attribute_description[3].binding = 0;
    attribute_description[3].location = 3;
    attribute_description[3].offset = offsetof(Vertex, tangent);
    attribute_description[3].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[4].binding = 0;
    attribute_description[4].location = 4;
    attribute_description[4].offset = offsetof(Vertex, bitangent);
    attribute_description[4].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribute_description[5].binding = 1;
    attribute_description[5].location = 5;
    attribute_description[5].offset = 0;
    attribute_description[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[6].binding = 1;
    attribute_description[6].location = 6;
    attribute_description[6].offset = sizeof(glm::vec4);
    attribute_description[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[7].binding = 1;
    attribute_description[7].location = 7;
    attribute_description[7].offset = sizeof(glm::vec4) * 2;
    attribute_description[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;

    attribute_description[8].binding = 1;
    attribute_description[8].location = 8;
    attribute_description[8].offset = sizeof(glm::vec4) * 3;
    attribute_description[8].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    return (attribute_description);
}