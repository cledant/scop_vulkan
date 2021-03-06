#ifndef SCOP_VULKAN_ASSIMPMODELLOADER_HPP
#define SCOP_VULKAN_ASSIMPMODELLOADER_HPP

#include "Mesh.hpp"

#include "assimp/scene.h"

void assimpLoadModel(char const *model_path,
                     std::vector<Vertex> &vertex_list,
                     std::vector<uint32_t> &indices_list,
                     std::vector<Mesh> &mesh_list);
void assimpLoadNode(aiNode *node,
                    aiScene const *scene,
                    std::unordered_map<Vertex, uint32_t> &unique_vertices,
                    std::vector<Vertex> &vertex_list,
                    std::vector<uint32_t> &indices_list,
                    std::vector<Mesh> &mesh_list);
void assimpLoadMesh(aiMesh *mesh,
                    aiScene const *scene,
                    std::unordered_map<Vertex, uint32_t> &unique_vertices,
                    std::vector<Vertex> &vertex_list,
                    std::vector<uint32_t> &indices_list,
                    std::vector<Mesh> &mesh_list);
Material assimpLoadMaterial(aiMesh *mesh, aiScene const *scene);
std::string assimpGetTextureName(aiMaterial *mat, aiTextureType type);

#endif // SCOP_VULKAN_ASSIMPMODELLOADER_HPP
