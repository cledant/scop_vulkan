#ifndef SCOP_VULKAN_MESH_HPP
#define SCOP_VULKAN_MESH_HPP

#include <string>
#include <vector>

#include "glm/glm.hpp"

struct Vertex final
{
    Vertex() = default;
    ~Vertex() = default;

    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 tex_coords{};
    glm::vec3 tangent{};
    glm::vec3 bitangent{};
};

struct Material final
{
    Material() = default;
    ~Material() = default;

    void printMaterial() const;

    glm::vec3 ambient = glm::vec3(1.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);
    float shininess = 2.0f;
    std::string material_name;
    std::string tex_ambient_name;
    std::string tex_diffuse_name;
    std::string tex_specular_name;
    std::string tex_normal_name;
    std::string tex_alpha_name;
};

struct Mesh final
{
    Mesh() = default;
    ~Mesh() = default;

    void printMesh() const;

    std::vector<Vertex> vertex_list;
    std::vector<uint32_t> indices;
    Material material{};
    glm::vec3 center{};
    glm::vec3 min_point{};
    glm::vec3 max_point{};
    std::string mesh_name;
};

#endif
