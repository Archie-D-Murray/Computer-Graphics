#pragma once

#include <glm/glm.hpp>
#include <string>
#include "maths.hpp"
#include "model.hpp"

class Object {
public:
  glm::vec3 position;
  glm::vec3 scale;
  Quaternion rotation;
  std::string name = "Object";
  Model* model = nullptr;
  glm::vec3 tint = glm::vec3(1.0f);

  Object(const glm::vec3& position, const glm::vec3& scale, const Quaternion& rotation, const char* name, Model* model);

  glm::mat4 modelMat();
  void draw(uint32_t shaderID);
};
