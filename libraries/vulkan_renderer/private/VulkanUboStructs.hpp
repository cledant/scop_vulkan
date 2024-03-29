#ifndef SCOP_VULKAN_VULKANUBOSTRUCTS_HPP
#define SCOP_VULKAN_VULKANUBOSTRUCTS_HPP

struct SystemUbo final
{
    alignas(16) glm::mat4 view_proj{};
};

struct ModelPipelineUbo final
{
    alignas(16) glm::vec3 diffuse_color{};
    alignas(16) glm::vec3 specular_color{};
    alignas(16) float shininess{};
};

#endif // SCOP_VULKAN_VULKANUBOSTRUCTS_HPP
