#include "maths.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include <glm/geometric.hpp>
#include <glm/gtx/quaternion.hpp>

Quaternion::Quaternion() : w(1), x(0), y(0), z(0) {}
Quaternion::Quaternion(float w, float x, float y, float z) :
    w(w), x(x), y(y), z(z)
{}
Quaternion::Quaternion(float pitch, float yaw) {
    float halfPitch = 0.5f * pitch;
    float halfYaw = 0.5f * yaw;

    float sinPitch = sinf(halfPitch);
    float cosPitch = cosf(halfPitch);
    float sinYaw   = sinf(halfYaw);
    float cosYaw   = cosf(halfYaw);

    // Quaternion for yaw (Y axis)
    float wYaw = cosYaw;
    float xYaw = 0.0f;
    float yYaw = sinYaw;
    float zYaw = 0.0f;

    // Quaternion for pitch (X axis)
    float wPitch = cosPitch;
    float xPitch = sinPitch;
    float yPitch = 0.0f;
    float zPitch = 0.0f;

    // Combine: q = yaw * pitch
    this->w = wYaw * wPitch - xYaw * xPitch - yYaw * yPitch - zYaw * zPitch;
    this->x = wYaw * xPitch + xYaw * wPitch + yYaw * zPitch - zYaw * yPitch;
    this->y = wYaw * yPitch - xYaw * zPitch + yYaw * wPitch + zYaw * xPitch;
    this->z = wYaw * zPitch + xYaw * yPitch - yYaw * xPitch + zYaw * wPitch;
}

glm::mat4 Quaternion::matrix() {
    // float s = 2.0f / (w * w + x * x + y * y + z * z);
    // float xs = x * s,  ys = y * s,  zs = z * s;
    // float xx = x * xs, xy = x * ys, xz = x * zs;
    // float yy = y * ys, yz = y * zs, zz = z * zs;
    // float xw = w * xs, yw = w * ys, zw = w * zs;

    glm::mat4 rotate = glm::mat4(1.0f);
    rotate[0][0] = (w * w) + (x * x) - (y * y) - (z * z);
    rotate[0][1] = (2 * x * y) + (2 * w * z);
    rotate[0][2] = (2 * x * z) - (2 * w * y);
    rotate[0][3] = 0.0f;

    rotate[1][0] = (2 * x * y) - (2 * w * z);
    rotate[1][1] = (w * w) - (x * x) + (y * y) - (z * z);
    rotate[1][2] = (2 * y * z) + (2 * w * x);
    rotate[1][3] = 0.0f;

    rotate[2][0] = (2 * x * z) + (2 * w * y);
    rotate[2][1] = (2 * y * z) - (2 * w * x);
    rotate[2][2] = (w * w) - (x * x) - (y * y) + (z * z);
    rotate[2][3] = 0.0f;

    rotate[3][0] = 0.0f;
    rotate[3][1] = 0.0f;
    rotate[3][2] = 0.0f;
    rotate[3][3] = 1.0f;
    return rotate;
}

Quaternion Quaternion::slerp(Quaternion from, Quaternion to, float t) {
    float cosTheta = from.w * to.w + from.x * to.x + from.y * to.y + from.z * to.z;

    if (cosTheta > 0.9999f) {
        return to;
    }

    if (cosTheta < 0) {
        to = Quaternion(-to.w, -to.x, -to.y, -to.z);
        cosTheta *= -1.0f;
    }

    Quaternion rotation = Quaternion();

    float theta = acosf(cosTheta);
    float a = sinf((1.0f - t) * theta) / sinf(theta);
    float b = sin(t * theta) / sin(theta);

    rotation.w = a * from.w + b * to.w;
    rotation.x = a * from.x + b * to.x;
    rotation.y = a * from.y + b * to.y;
    rotation.z = a * from.z + b * to.z;

    return rotation;
}

float Maths::radians(float degrees) {
    return degrees * DEG2RAD;
}

glm::mat4 Maths::scale(const glm::vec3 &scale) {
    glm::mat4 scaleMat = glm::mat4(1.0f);
    scaleMat[0][0] = scale.x;
    scaleMat[1][1] = scale.y;
    scaleMat[2][2] = scale.z;
    return scaleMat;
}

glm::mat4 Maths::rotate(glm::vec3 axis, float angle) {
    axis = glm::normalize(axis);
    float c = cosf(0.5f * angle);
    float s = sinf(0.5f * angle);
    Quaternion rotation = Quaternion(c, s * axis.x, s * axis.y, s * axis.z);
    return rotation.matrix();
}

glm::mat4 Maths::translate(const glm::vec3 &translation) {
    glm::mat4 translateMat = glm::mat4(1.0f);
    translateMat[3][0] = translation.x;
    translateMat[3][1] = translation.y;
    translateMat[3][2] = translation.z;
    return translateMat;
}

glm::mat4 Maths::transpose(const glm::mat4& in) {
    glm::mat4 out = glm::mat4(0.0f);

    out[0][0] = in[0][0]; out[0][1] = in[1][0]; out[0][2] = in[2][0]; out[0][3] = in[3][0];
    out[1][0] = in[0][1]; out[1][1] = in[1][1]; out[1][2] = in[2][1]; out[1][3] = in[3][1];
    out[2][0] = in[0][2]; out[2][1] = in[1][2]; out[2][2] = in[2][2]; out[2][3] = in[3][2];
    out[3][0] = in[0][3]; out[3][1] = in[1][3]; out[3][2] = in[2][3]; out[3][3] = in[3][3];

    return out;
}

float Maths::yaw(const glm::vec3& angles) {
    return angles.y;
}

float Maths::pitch(const glm::vec3& angles) {
    return angles.x;
}

float Maths::roll(const glm::vec3& angles) {
    return angles.z;
}

glm::mat4 Maths::lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp) {
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec3 forward = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
    glm::vec3 up = glm::cross(right, forward);
    glm::vec3 negativePosition = -1.0f * position;
    view[0][0] = right.x;
    view[0][1] = right.y;
    view[0][2] = right.z;
    view[0][3] = glm::dot(negativePosition, right);
    view[1][0] = up.x;
    view[1][1] = up.y;
    view[1][2] = up.z;
    view[1][3] = glm::dot(negativePosition, up);
    view[2][0] = -1.0f * forward.x;
    view[2][1] = -1.0f * forward.y;
    view[2][2] = -1.0f * forward.z;
    view[2][3] = glm::dot(negativePosition, forward);
    view[3][0] = 0.0f;
    view[3][1] = 0.0f;
    view[3][2] = 0.0f;
    view[3][3] = 1.0f;
    return view;
}

glm::mat4 Maths::perspective(float fov, float aspect, float near, float far) {
    glm::perspective(fov, aspect, near, far);
    const float top = near * tanf(0.5f * fov);
    const float right = aspect * top;

    glm::mat4 perspective = glm::mat4(0.0f);
    perspective[0][0] = near / right;
    perspective[1][1] = near / top;
    perspective[2][2] = -(far + near)/(far - near);
    perspective[2][3] = -1;
    perspective[3][2] = -(2.0f * far * near)/(far - near);
    return perspective;
}

float Maths::magnitude(const glm::vec3& vector) {
    return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

float Maths::magnitude(const glm::vec2& vector) {
    return sqrtf(vector.x * vector.x + vector.y * vector.y);
}

float Maths::sqrMagnitude(const glm::vec3& vector) {
    return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

float Maths::sqrMagnitude(const glm::vec2& vector) {
    return vector.x * vector.x + vector.y * vector.y;
}

glm::vec3 Maths::normalize(const glm::vec3& vector) {
    return vector / magnitude(vector);
}

glm::vec2 Maths::normalize(const glm::vec2& vector) {
    return vector / magnitude(vector);
}

glm::vec3 Maths::cross(const glm::vec3& a, const glm::vec3& b) {
    return glm::vec3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y);
}

float Maths::dot(const glm::vec3& a, const glm::vec3& b) {
    return (a.x * b.x + a.y * b.y + a.z + b.z) / magnitude(a) * magnitude(b);
}

bool Maths::inRange(const glm::vec3& centre, const glm::vec3& point, float distance) {
    return Maths::sqrMagnitude(point - centre) <= distance * distance;
}

Quaternion Maths::lookDirection(const glm::vec3& forward, const glm::vec3& up) {
    glm::vec3 dir = normalize(forward);
    float pitch = asinf(-dir.y);
    float yaw = atan2f(dir.x, -dir.z);

    return Quaternion(pitch, yaw);
}
