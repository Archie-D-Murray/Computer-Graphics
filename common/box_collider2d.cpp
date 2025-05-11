#include "box_collider2d.hpp"

BoxCollider2D::BoxCollider2D(const glm::vec3& position, const glm::vec2& size) :
    position(position),
    size(size)
{}

void BoxCollider2D::updatePosition(const glm::vec3& newPosition) {
    position = newPosition;
}
bool BoxCollider2D::isTouching(const BoxCollider2D& a, const BoxCollider2D& b) {
    if (a.front() > b.back() || a.back() < b.front()) {
        return false;
    }

    if (a.right() < b.left() || a.left() > b.right()) {
        return false;
    }

    return true;
}

float BoxCollider2D::front() const {
    return position.z - size.y * 0.5f;
}

float BoxCollider2D::back() const {
    return position.z + size.y * 0.5f;
}

float BoxCollider2D::right() const {
    return position.z + size.x * 0.5f;
}

float BoxCollider2D::left() const {
    return position.z - size.x * 0.5f;
}
