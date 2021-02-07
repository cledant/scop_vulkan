#include "Model.hpp"

#include <cstring>

#include "AssimpModelLoader.hpp"

Model::Model(const std::string &model_path)
{
    loadModel(model_path.c_str());
}

Model::Model(const char *model_path)
{
    loadModel(model_path);
}

void
Model::loadModel(const std::string &model_path)
{
    auto pos = model_path.find_last_of('/');
    if (pos == std::string::npos) {
        _directory = ".";
    } else {
        _directory = model_path.substr(0, pos);
    }
    assimpLoadModel(model_path.c_str(), _mesh_list);
}

void
Model::loadModel(const char *model_path)
{
    assert(model_path);
    auto pos = std::strrchr(model_path, '/');
    if (!pos) {
        _directory = ".";
    } else {
        uintptr_t size = pos - model_path;
        _directory = std::string(model_path, size);
    }
    assimpLoadModel(model_path, _mesh_list);
}

void
Model::printModel() const
{
    for (auto const &it : _mesh_list) {
        it.printMesh();
    }
}

std::vector<Mesh> const &
Model::getMeshList() const
{
    return (_mesh_list);
}

glm::vec3 const &
Model::getCenter() const
{
    return (_center);
}

std::string const &
Model::getDirectory() const
{
    return (_directory);
}
