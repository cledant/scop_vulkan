#ifndef SCOP_VULKAN_TESTMODELINSTANCING_HPP
#define SCOP_VULKAN_TESTMODELINSTANCING_HPP

#include <array>

#include <vulkan/vulkan.h>

#include "glm/glm.hpp"

struct ModelUbo
{
    glm::mat4 view_proj;
};

struct TestVertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;

    static std::array<VkVertexInputBindingDescription, 2>
    getBindingDescription()
    {
        std::array<VkVertexInputBindingDescription, 2> binding_description{};
        binding_description[0].binding = 0;
        binding_description[0].stride = sizeof(TestVertex);
        binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        binding_description[1].binding = 1;
        binding_description[1].stride = sizeof(glm::mat4);
        binding_description[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return (binding_description);
    }

    static std::array<VkVertexInputAttributeDescription, 7>
    getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 7>
          attribute_description{};

        attribute_description[0].binding = 0;
        attribute_description[0].location = 0;
        attribute_description[0].offset = 0;
        attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;

        attribute_description[1].binding = 0;
        attribute_description[1].location = 1;
        attribute_description[1].offset = offsetof(TestVertex, color);
        attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;

        attribute_description[2].binding = 0;
        attribute_description[2].location = 2;
        attribute_description[2].offset = offsetof(TestVertex, tex_coord);
        attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;

        attribute_description[3].binding = 1;
        attribute_description[3].location = 3;
        attribute_description[3].offset = 0;
        attribute_description[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;

        attribute_description[4].binding = 1;
        attribute_description[4].location = 4;
        attribute_description[4].offset = sizeof(glm::vec4);
        attribute_description[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;

        attribute_description[5].binding = 1;
        attribute_description[5].location = 5;
        attribute_description[5].offset = sizeof(glm::vec4) * 2;
        attribute_description[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;

        attribute_description[6].binding = 1;
        attribute_description[6].location = 6;
        attribute_description[6].offset = sizeof(glm::vec4) * 3;
        attribute_description[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        return (attribute_description);
    }
};

// Test triangle
[[maybe_unused]] constexpr std::array<TestVertex, 4> const
  TEST_TRIANGLE_VERTICIES = { {
    { { -0.5f, 1.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 0.5f, 1.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
    { { 0.5f, 1.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    { { -0.5f, 1.0f, 0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
  } };

// TODO : Used for memory alignment => do more dynamic stuff
[[maybe_unused]] constexpr std::array<uint32_t, 16> const
  TEST_TRIANGLE_INDICES = {
      0, 1, 2, 2, 3, 0,
  };

[[maybe_unused]] constexpr std::array<glm::vec3, 4> TEST_TRIANGLE_POS = { {
  { 2.0f, 0.0f, 2.0f },
  { -2.0f, 0.0f, 2.0f },
  { -2.0f, 0.0f, -2.0f },
  { 2.0f, 0.0f, -2.0f },
} };

#endif // SCOP_VULKAN_TESTMODELINSTANCING_HPP
