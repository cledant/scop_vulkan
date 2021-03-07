#include "ModelInstanceInfo.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/gtc/matrix_transform.hpp"

glm::mat4
computeInstanceMatrix(glm::vec3 const &modelCenter,
                      ModelInstanceInfo const &info)
{
    auto instance_matrix = glm::mat4(1.0f);
    instance_matrix = glm::scale(instance_matrix, info.scale);
    instance_matrix = glm::translate(instance_matrix, -modelCenter);

    instance_matrix =
      glm::rotate(instance_matrix, info.pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    instance_matrix =
      glm::rotate(instance_matrix, info.yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    instance_matrix =
      glm::rotate(instance_matrix, info.roll, glm::vec3(0.0f, 0.0f, 1.0f));
    instance_matrix = glm::translate(instance_matrix, info.position);

    return (instance_matrix);
}
