#ifndef SCOP_VULKAN_VULKANUBOSTRUCTS_HPP
#define SCOP_VULKAN_VULKANUBOSTRUCTS_HPP

struct SystemUbo
{
    alignas(16) glm::mat4 view_proj{};
};

struct ModelPipelineUbo
{
    alignas(16) glm::vec3 diffuse_color{};
    alignas(16) glm::vec3 specular_color{};
    alignas(16) float shininess;
    alignas(16) glm::vec4 unused{};
};

#endif // SCOP_VULKAN_VULKANUBOSTRUCTS_HPP
