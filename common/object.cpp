#include "object.hpp"
#include "glm/gtc/type_ptr.hpp"

glm::mat4 Object::modelMat() {
  return Maths::translate(position) * rotation.matrix() * Maths::scale(scale);
}
void Object::draw(uint32_t shaderID) {
  if (model) {
    glUniform3fv(glGetUniformLocation(shaderID, "modelTint"), 1, glm::value_ptr(tint));
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
