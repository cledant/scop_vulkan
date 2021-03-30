#include "Model.hpp"

#include <cstring>

#include "fmt/core.h"

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
    _model_path = model_path;
    auto pos = model_path.find_last_of('/');
    if (pos == std::string::npos) {
        _directory = ".";
    } else {
        _directory = model_path.substr(0, pos);
    }
    assimpLoadModel(
      model_path.c_str(), _vertex_list, _indices_list, _mesh_list);
    _compute_min_max_points_and_center();
}

void
Model::loadModel(const char *model_path)
{
    assert(model_path);
    _model_path = model_path;
    auto pos = std::strrchr(model_path, '/');
    if (!pos) {
        _directory = ".";
    } else {
        uintptr_t size = pos - model_path;
        _directory = std::string(model_path, size);
    }
    assimpLoadModel(model_path, _vertex_list, _indices_list, _mesh_list);
    _compute_min_max_points_and_center();
}

void
Model::printModel() const
{
    fmt::print("MODEL:\n");
    fmt::print("Model Path: {}\n", _model_path);
    fmt::print("Model Directory: {}\n", _directory);
    fmt::print("Model Nb Faces: {}\n", _nb_faces);
    fmt::print("Model Min Point: ( {} | {} | {} )\n",
               _min_point.x,
               _min_point.y,
               _min_point.z);
    fmt::print("Model Max Point: ( {} | {} | {} )\n",
               _max_point.x,
               _max_point.y,
               _max_point.z);
    fmt::print(
      "Model Center: ( {} | {} | {} )\n", _center.x, _center.y, _center.z);
    for (auto const &it : _mesh_list) {
        it.printMesh();
    }
    fmt::print("===== END MODEL =====\n");
}

ModelInfo
Model::getModelInfo() const
{
    ModelInfo info = { static_cast<uint32_t>(_vertex_list.size()),
                       static_cast<uint32_t>(_indices_list.size()),
                       _nb_faces };
    return (info);
}

std::vector<Vertex> const &
Model::getVertexList() const
{
    return (_vertex_list);
}

std::vector<uint32_t> const &
Model::getIndicesList() const
{
    return (_indices_list);
}

std::vector<Mesh> const &
Model::getMeshList() const
{
    return (_mesh_list);
}

std::string const &
Model::getDirectory() const
{
    return (_directory);
}

glm::vec3 const &
Model::getCenter() const
{
    return (_center);
}

void
Model::_compute_min_max_points_and_center()
{
    if (!_mesh_list.empty()) {
        _min_point = _mesh_list[0].min_point;
        _max_point = _mesh_list[0].max_point;
        for (auto const &it : _mesh_list) {
            // Min points
            _min_point.x =
              (_min_point.x > it.min_point.x) ? it.min_point.x : _min_point.x;
            _min_point.y =
              (_min_point.y > it.min_point.y) ? it.min_point.y : _min_point.y;
            _min_point.z =
              (_min_point.z > it.min_point.z) ? it.min_point.z : _min_point.z;

            // Max points
            _max_point.x =
              (_max_point.x < it.max_point.x) ? it.max_point.x : _max_point.x;
            _max_point.y =
              (_max_point.y < it.max_point.y) ? it.max_point.y : _max_point.y;
            _max_point.z =
              (_max_point.z < it.max_point.z) ? it.max_point.z : _max_point.z;

            _nb_faces += it.nb_faces;
        }

        // Center
        _center = { (_min_point.x + _max_point.x) / 2.0f,
                    (_min_point.y + _max_point.y) / 2.0f,
                    (_min_point.z + _max_point.z) / 2.0f };
    }
}