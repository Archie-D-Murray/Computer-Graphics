#include "object.hpp"

glm::mat4 Object::modelMat() {
  return Maths::translate(position) * rotation.matrix() * Maths::scale(scale);
}
void Object::draw(uint32_t shaderID) {
  if (model) {
    model->draw(shaderID);
  }
}

Object::Object(const glm::vec3& position, const glm::vec3& scale, const Quaternion& rotation, const char* name, Model* model) :
  position(position),
  scale(scale),
  rotation(rotation),
  name(name),
  model(model)
{}
