#include "AssimpModelLoader.hpp"

#include <cassert>
#include <stdexcept>

#include "fmt/core.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

void
assimpLoadModel(char const *model_path,
                std::vector<Vertex> &vertex_list,
                std::vector<uint32_t> &indices_list,
                std::vector<Mesh> &mesh_list)
{
    assert(model_path);
    Assimp::Importer importer{};
    std::unordered_map<Vertex, uint32_t> unique_vertices{};

    aiScene const *scene =
      importer.ReadFile(model_path,
                        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                          aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode)
        throw std::runtime_error("AssimpLoader: Failed to load: " +
                                 std::string(model_path));
    assimpLoadNode(scene->mRootNode,
                   scene,
                   unique_vertices,
                   vertex_list,
                   indices_list,
                   mesh_list);
}

void
assimpLoadNode(aiNode *node,
               aiScene const *scene,
               std::unordered_map<Vertex, uint32_t> &unique_vertices,
               std::vector<Vertex> &vertex_list,
               std::vector<uint32_t> &indices_list,
               std::vector<Mesh> &mesh_list)
{
    assert(node && scene);
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        assimpLoadMesh(scene->mMeshes[node->mMeshes[i]],
                       scene,
                       unique_vertices,
                       vertex_list,
                       indices_list,
                       mesh_list);
    }
    for (size_t j = 0; j < node->mNumChildren; ++j) {
        assimpLoadNode(node->mChildren[j],
                       scene,
                       unique_vertices,
                       vertex_list,
                       indices_list,
                       mesh_list);
    }
}

void
assimpLoadMesh(aiMesh *mesh,
               aiScene const *scene,
               std::unordered_map<Vertex, uint32_t> &unique_vertices,
               std::vector<Vertex> &vertex_list,
               std::vector<uint32_t> &indices_list,
               std::vector<Mesh> &mesh_list)
{
    assert(mesh && scene);
    Mesh loaded_mesh;
    loaded_mesh.indices_offset = unique_vertices.size();

    // Init Min / Max point
    if (mesh->mNumVertices) {
        loaded_mesh.min_point = { mesh->mVertices[0].x,
                                  mesh->mVertices[0].y,
                                  mesh->mVertices[0].z };
        loaded_mesh.max_point = loaded_mesh.min_point;
    }

    // Vertices
    uint32_t model_indices = unique_vertices.size();
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        Vertex loaded_vertex{};

        loaded_vertex.position = { mesh->mVertices[i].x,
                                   mesh->mVertices[i].y,
                                   mesh->mVertices[i].z };
        loaded_vertex.normal = { mesh->mNormals[i].x,
                                 mesh->mNormals[i].y,
                                 mesh->mNormals[i].z };
        if (mesh->HasTextureCoords(0)) {
            loaded_vertex.tex_coords = { mesh->mTextureCoords[0][i].x,
                                         mesh->mTextureCoords[0][i].y };
            loaded_vertex.tangent = { mesh->mTangents[i].x,
                                      mesh->mTangents[i].y,
                                      mesh->mTangents[i].z };
            loaded_vertex.bitangent = { mesh->mBitangents[i].x,
                                        mesh->mBitangents[i].y,
                                        mesh->mBitangents[i].z };
        }
        if (!unique_vertices.contains(loaded_vertex)) {
            unique_vertices[loaded_vertex] = model_indices;
            vertex_list.emplace_back(loaded_vertex);
            ++model_indices;
            ++loaded_mesh.nb_indices;
        }

        // Indices
        indices_list.emplace_back(unique_vertices[loaded_vertex]);

        // Min points
        loaded_mesh.min_point.x =
          (loaded_mesh.min_point.x > mesh->mVertices[i].x)
            ? mesh->mVertices[i].x
            : loaded_mesh.min_point.x;
        loaded_mesh.min_point.y =
          (loaded_mesh.min_point.y > mesh->mVertices[i].y)
            ? mesh->mVertices[i].y
            : loaded_mesh.min_point.y;
        loaded_mesh.min_point.z =
          (loaded_mesh.min_point.z > mesh->mVertices[i].z)
            ? mesh->mVertices[i].z
            : loaded_mesh.min_point.z;

        // Max points
        loaded_mesh.max_point.x =
          (loaded_mesh.max_point.x < mesh->mVertices[i].x)
            ? mesh->mVertices[i].x
            : loaded_mesh.max_point.x;
        loaded_mesh.max_point.y =
          (loaded_mesh.max_point.y < mesh->mVertices[i].y)
            ? mesh->mVertices[i].y
            : loaded_mesh.max_point.y;
        loaded_mesh.max_point.z =
          (loaded_mesh.max_point.z < mesh->mVertices[i].z)
            ? mesh->mVertices[i].z
            : loaded_mesh.max_point.z;
    }

    // Center
    loaded_mesh.center = {
        (loaded_mesh.min_point.x + loaded_mesh.max_point.x) / 2.0f,
        (loaded_mesh.min_point.y + loaded_mesh.max_point.y) / 2.0f,
        (loaded_mesh.min_point.z + loaded_mesh.max_point.z) / 2.0f
    };

    // Other
    loaded_mesh.material = assimpLoadMaterial(mesh, scene);
    loaded_mesh.mesh_name = mesh->mName.C_Str();
    loaded_mesh.nb_faces = mesh->mNumFaces;

    mesh_list.emplace_back(loaded_mesh);
}

Material
assimpLoadMaterial(aiMesh *mesh, aiScene const *scene)
{
    assert(mesh);
    Material mat{};
    auto mesh_mat = scene->mMaterials[mesh->mMaterialIndex];

    aiColor3D tmp;
    float tmp_shininess = 0.0f;

    // Colors
    if (mesh_mat->Get(AI_MATKEY_COLOR_AMBIENT, tmp) == AI_SUCCESS) {
        mat.ambient = glm::vec3(tmp.r, tmp.g, tmp.b);
    }
    if (mesh_mat->Get(AI_MATKEY_COLOR_DIFFUSE, tmp) == AI_SUCCESS) {
        mat.diffuse = glm::vec3(tmp.r, tmp.g, tmp.b);
    }
    if (mesh_mat->Get(AI_MATKEY_COLOR_SPECULAR, tmp) == AI_SUCCESS) {
        mat.specular = glm::vec3(tmp.r, tmp.g, tmp.b);
    }
    if (mesh_mat->Get(AI_MATKEY_SHININESS, tmp_shininess) == AI_SUCCESS) {
        mat.shininess = tmp_shininess;
    }

    // Textures
    mat.tex_ambient_name =
      assimpGetTextureName(mesh_mat, aiTextureType_AMBIENT);
    mat.tex_diffuse_name =
      assimpGetTextureName(mesh_mat, aiTextureType_DIFFUSE);
    mat.tex_specular_name =
      assimpGetTextureName(mesh_mat, aiTextureType_SPECULAR);
    mat.tex_normal_name = assimpGetTextureName(mesh_mat, aiTextureType_HEIGHT);
    mat.tex_alpha_name = assimpGetTextureName(mesh_mat, aiTextureType_OPACITY);

    // Name
    aiString mat_name;
    if (mesh_mat->Get(AI_MATKEY_NAME, mat_name) == AI_SUCCESS) {
        mat.material_name = mat_name.C_Str();
    }
    return (mat);
}

std::string
assimpGetTextureName(aiMaterial *mat, aiTextureType type)
{
    assert(mat);
    aiString str;

    if (mat->GetTextureCount(type)) {
        mat->GetTexture(type, 0, &str);
    }
    return (str.C_Str());
}
