#pragma once

#include "maths.hpp"

class Camera {

const glm::vec3 worldUp = { 0, 1, 0 };

public:
    float fov = glm::radians(45.0f);
    float aspect = 1024.0f / 768.0f;
    float near = 0.2f;
    float far = 100.0f;

    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 forward;
    float yaw = Maths::radians(-90.0f);
    float pitch = 0;
    float roll = 0;

    Quaternion orientation;

    glm::mat4 view;
    glm::mat4 projection;

    Camera(const glm::vec3 eye, const glm::vec3 target);
    void quaternionCamera(float dt);
    void lookAt(const glm::vec3 point);
};
