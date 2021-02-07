#include "Mesh.hpp"

#include <iostream>

void
Material::printMaterial() const
{
    std::cout << "\tMATERIAL: " << std::endl;
    std::cout << "\t\tMaterial Name: " << material_name << std::endl;
    std::cout << "\t\tAmbient Color: "
              << "( " << ambient.x << " | " << ambient.y << " | " << ambient.z
              << " )" << std::endl;
    std::cout << "\t\tDiffuse Color: "
              << "( " << diffuse.x << " | " << diffuse.y << " | " << diffuse.z
              << " )" << std::endl;
    std::cout << "\t\tSpecular Color: "
              << "( " << specular.x << " | " << specular.y << " | "
              << specular.z << " )" << std::endl;
    std::cout << "\t\tShininess: " << shininess << std::endl;
    std::cout << "\t\tAmbient Texture Name: " << tex_ambient_name << std::endl;
    std::cout << "\t\tDiffuse Texture Name: " << tex_diffuse_name << std::endl;
    std::cout << "\t\tSpecular Texture Name: " << tex_specular_name
              << std::endl;
    std::cout << "\t\tNormal Texture Name: " << tex_normal_name << std::endl;
    std::cout << "\t\tAlpha Texture Name: " << tex_alpha_name << std::endl;
}

void
Mesh::printMesh() const
{
    std::cout << "\tMESH:" << std::endl;
    std::cout << "\tMesh Name: " << mesh_name << std::endl;
    material.printMaterial();
    std::cout << "\tNb Vertices: " << vertex_list.size() << std::endl;
    std::cout << "\tNb indices: " << indices.size() << std::endl;
    std::cout << "----- END MESH -----" << std::endl;
}
