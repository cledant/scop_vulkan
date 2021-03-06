#ifndef SCOP_VULKAN_MESH_HPP
#define SCOP_VULKAN_MESH_HPP

#include <string>
#include <vector>
#include <functional>

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex final
{
    bool operator==(Vertex const &rhs) const = default;

    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 tex_coords{};
    glm::vec3 tangent{};
    glm::vec3 bitangent{};
};

struct Material final
{
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
    void printMesh() const;

    Material material{};
    glm::vec3 center{};
    glm::vec3 min_point{};
    glm::vec3 max_point{};
    uint32_t nb_faces{};
    uint32_t nb_indices{};
    uint32_t indices_offset{};
    std::string mesh_name;
};

size_t constexpr hash_combine(size_t h1, size_t h2)
{
    return (h1 ^ (h2 << 1));
};

namespace std {
template<>
struct hash<Vertex> final
{
    size_t
    operator()(Vertex const &vertex) const noexcept
    {
        using namespace glm;
        auto h1 = hash_combine(hash<vec3>()(vertex.position),
                               hash<vec3>()(vertex.normal)) >>
                  1;
        auto h2 = hash_combine(h1, hash<vec2>()(vertex.tex_coords)) >> 1;
        auto h3 = hash_combine(h2, hash<vec3>()(vertex.tangent)) >> 1;
        auto h4 = hash_combine(h3, hash<vec3>()(vertex.bitangent));
        return (h4);
    }
};
}

#endif
