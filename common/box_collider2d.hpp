#pragma once
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <iostream>

constexpr float MIN_DOT_PRODUCT = 0.5f;

class BoxCollider2D {
public:
    glm::vec3 position;
    glm::vec2 size;

    BoxCollider2D(const glm::vec3& position, const glm::vec2& size);
    void updatePosition(const glm::vec3& newPosition);
    static bool isTouching(const BoxCollider2D& a, const BoxCollider2D& b);

    inline float front() const;
    inline float back() const;
    inline float right() const;
    inline float left() const;
};

std::ostream& operator<<(std::ostream& os, const BoxCollider2D& box);
