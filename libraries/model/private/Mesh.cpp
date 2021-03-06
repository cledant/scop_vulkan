#include "Mesh.hpp"

#include "fmt/core.h"

void
Material::printMaterial() const
{
    fmt::print("\tMATERIAL:\n");
    fmt::print("\t\tMaterial Name: {}\n", material_name);
    fmt::print(
      "\t\tAmbient Color: ( {} | {} | {} )\n", ambient.x, ambient.y, ambient.z);
    fmt::print(
      "\t\tDiffuse Color: ( {} | {} | {} )\n", diffuse.x, diffuse.y, diffuse.z);
    fmt::print("\t\tSpecular Color: ( {} | {} | {} )\n",
               specular.x,
               specular.y,
               specular.z);
    fmt::print("\t\tShininess: {}\n", shininess);
    fmt::print("\t\tAmbient Texture Name: {}\n", tex_ambient_name);
    fmt::print("\t\tDiffuse Texture Name: {}\n", tex_diffuse_name);
    fmt::print("\t\tSpecular Texture Name: {}\n", tex_specular_name);
    fmt::print("\t\tNormal Texture Name: {}\n", tex_normal_name);
    fmt::print("\t\tAlpha Texture Name: {}\n", tex_alpha_name);
}

void
Mesh::printMesh() const
{
    fmt::print("\tMESH:\n");
    fmt::print("\tMesh Name: {}\n", mesh_name);
    material.printMaterial();
    fmt::print("\t\tMin Point: ( {} | {} | {} )\n",
               min_point.x,
               min_point.y,
               min_point.z);
    fmt::print("\t\tMax Point: ( {} | {} | {} )\n",
               max_point.x,
               max_point.y,
               max_point.z);
    fmt::print("\t\tCenter: ( {} | {} | {} )\n", center.x, center.y, center.z);
    fmt::print("\tNb Faces: {}\n", nb_faces);
    fmt::print("\tNb Indices: {}\n", nb_indices);
    fmt::print("\tIndices buffer offset: {}\n", indices_offset);
    fmt::print("----- END MESH -----\n");
}
