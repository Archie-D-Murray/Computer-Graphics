#pragma once
#include <cassert>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static constexpr float DEG2RAD = M_PI / 180.0f;

class Quaternion {
public:
    float w, x, y, z;

    Quaternion();
    Quaternion(float w, float x, float y, float z);
    Quaternion(float pitch, float yaw);

    glm::mat4 matrix();
    static Quaternion slerp(Quaternion from, Quaternion to, float t);
};

namespace Maths {
    float radians(float degrees);

    glm::mat4 scale(const glm::vec3 &scale);

    glm::mat4 rotate(glm::vec3 axis, float angle);

    glm::mat4 translate(const glm::vec3 &translation);

    glm::mat4 lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp);
    glm::mat4 perspective(float fov, float aspect, float near, float far);
    glm::mat4 transpose(const glm::mat4& in);

    float yaw(const glm::vec3& angles);
    float pitch(const glm::vec3& angles);
    float roll(const glm::vec3& angles);

    // Vector3 maths
    float magnitude(const glm::vec3& vector);
    float sqrMagnitude(const glm::vec3& vector);
    float dot(const glm::vec3& a, const glm::vec3& b);
    float dot(const glm::vec2& a, const glm::vec2& b);
    glm::vec3 cross(const glm::vec3& a, const glm::vec3& b);
    glm::vec3 normalize(const glm::vec3& vector);
    bool inRange(const glm::vec3& centre, const glm::vec3& point, float distance);

    // Vector2 Maths
    float magnitude(const glm::vec2& vector);
    float sqrMagnitude(const glm::vec2& vector);
    glm::vec2 normalize(const glm::vec2& vector);

    // Quaternion Maths:
    Quaternion lookDirection(const glm::vec3& forward, const glm::vec3& up);

    // Colour:
    float hueToRGB(float p, float q, float t);
    glm::vec3 hslToRGB(glm::vec3 hsl);
}
