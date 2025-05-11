#include "camera.hpp"
#include "maths.hpp"
#include <glm/glm.hpp>

Camera::Camera(const glm::vec3 position, const glm::vec3 target) :
    position(position)
{
    glm::vec3 dir = Maths::normalize(target - position);
    pitch = asinf(-dir.y);
    yaw = atan2f(dir.x, -dir.z);
    this->orientation = Quaternion(-pitch, yaw);
}

void Camera::lookAt(const glm::vec3 point) {
    glm::vec3 dir = Maths::normalize(point - position);
    pitch = asinf(-dir.y);
    yaw = atan2f(dir.x, -dir.z);
    orientation = Quaternion(-pitch, yaw);
    quaternionCamera(1.0f);
}

void Camera::quaternionCamera(float dt) {
    Quaternion orientation = Quaternion(pitch, -yaw);
    // orientation = Quaternion::slerp(orientation, rotation, dt * 12.0f);
    view = Maths::transpose(orientation.matrix()) * Maths::translate(-position);
    projection = Maths::perspective(fov, aspect, near, far);

    right   =  glm::vec3(view[0][0], view[1][0], view[2][0]);
    up      =  glm::vec3(view[0][1], view[1][1], view[2][1]);
    forward = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}
