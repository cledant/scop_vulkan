#ifndef SCOP_VULKAN_ASSIMPMODELLOADER_HPP
#define SCOP_VULKAN_ASSIMPMODELLOADER_HPP

#include "Mesh.hpp"

#include "assimp/scene.h"

void assimpLoadModel(char const *model_path, std::vector<Mesh> &mesh_list);
void assimpLoadNode(aiNode *node,
                    aiScene const *scene,
                    std::vector<Mesh> &mesh_list);
void assimpLoadMesh(aiMesh *mesh,
                    aiScene const *scene,
                    std::vector<Mesh> &mesh_list);
Material assimpLoadMaterial(aiMesh *mesh, aiScene const *scene);
std::string assimpGetTextureName(aiMaterial *mat, aiTextureType type);

#endif // SCOP_VULKAN_ASSIMPMODELLOADER_HPP
