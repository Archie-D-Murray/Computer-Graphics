#include "box_collider2d.hpp"
#include <iostream>

BoxCollider2D::BoxCollider2D(const glm::vec3& position, const glm::vec2& size) :
    position(position),
    size(size)
{}

void BoxCollider2D::updatePosition(const glm::vec3& newPosition) {
    position = newPosition;
}

/*
 * Player BoxCollider {
      Position: [-0.0456829, 0, 0.743522]
      Size: [1, 1]
      Extents:
          Left: 0.243522
          Right: 1.24352
          Back: 0.243522
          Front: 1.24352
      }
Teapot BoxCollider {
    Position: [0, 0, 0]
    Size: [2, 2]
    Extents:
        Left: -1
        Right: 1
        Back: -1
        Front: 1
}
 * */

bool BoxCollider2D::isTouching(const BoxCollider2D& a, const BoxCollider2D& b) {
    if (a.front() < b.back() || a.back() > b.front()) {
        return false;
    }

    if (a.right() < b.left() || a.left() > b.right()) {
        return false;
    }

    return true;
}

float BoxCollider2D::front() const {
    return position.z + size.y * 0.5f;
}

float BoxCollider2D::back() const {
    return position.z - size.y * 0.5f;
}

float BoxCollider2D::right() const {
    return position.x + size.x * 0.5f;
}

float BoxCollider2D::left() const {
    return position.x - size.x * 0.5f;
}

std::ostream& operator<<(std::ostream& os, const BoxCollider2D& box) {
    os << "BoxCollider {\n"
       << "  Position: [" << box.position.x << ", 0, " << box.position.z << "]\n"
       << "  Size: [" << box.size.x << ", " << box.size.y << "]\n"
       << "  Extents:\n"
       << "    Left: " << box.left() << "\n"
       << "    Right: " << box.right() << "\n"
       << "    Back: " << box.back() << "\n"
       << "    Front: " << box.front() << "\n"
       << "}";
    return os;
}
