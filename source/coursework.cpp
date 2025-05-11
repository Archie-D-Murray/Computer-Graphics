#include "glm/detail/type_vec.hpp"
#include <cstdio>
#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>
#include <common/object.hpp>
#include <common/box_collider2d.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

glm::vec2 mouseDelta;
glm::vec2 movementInput;

float lastFrame = 0.0f;
float deltaTime = 0.0f;

const float width = 1260;
const float height = 720;
const float cameraSpeed = 0.075f;
const float walkSpeed = 1.0f;

// Function prototypes
void keyboardInput(GLFWwindow *window);
void mouseInput(GLFWwindow *window);

Camera camera = Camera({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f});

int main(void) {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    getchar();
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context


  GLFWwindow *window;
  window = glfwCreateWindow((int) width, (int) height, "Computer Graphics", NULL, NULL);

  if (window == NULL) {
    fprintf(stderr, "Failed to open GLFW window.\n");
    getchar();
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Vsync

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return -1;
  }

  glEnable(GL_DEPTH_TEST);

  // Use back face culling
  glEnable(GL_CULL_FACE);

  // Ensure we can capture keyboard inputs
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Capture mouse inputs
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwPollEvents();
  glfwSetCursorPos(window, width * 0.5f, height * 0.5f);

  uint32_t shaderID = LoadShaders("./vertexShader.glsl", "./fragmentShader.glsl");
  uint32_t mvpID = glGetUniformLocation(shaderID, "MVP");
  uint32_t mvID = glGetUniformLocation(shaderID, "MV");
  uint32_t tintID = glGetUniformLocation(shaderID, "tint");

  Light lights;

  lights.addPointLight(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.1f, 0.02f); // Only this works!!

  std::vector<BoxCollider2D> colliders;
  std::vector<Object> objects;

  BoxCollider2D playerCollider = BoxCollider2D(camera.position, glm::vec2(1.0f));

  Model teapot = Model("../assets/teapot.obj");
  teapot.addTexture("../assets/bricks_diffuse.png", "diffuse");
  teapot.addTexture("../assets/bricks_normal.png", "normal");
  teapot.ka = 0.2f;
  teapot.kd = 0.7f;
  teapot.ks = 1.0f;
  teapot.Ns = 20.0f;

  fprintf(stdout, "Loaded models\n");

  objects.push_back(
    Object(
      glm::vec3 {0, 0, 0},
      glm::vec3 {1, 1, 1} * 0.25f,
      Quaternion(), // Identity quaternion
      "Teapot",
      &teapot
  ));

  colliders.push_back(BoxCollider2D({ 0, 0, 0 }, { 2, 2 }));

  camera.lookAt(objects[0].position);

  float acc = 0;

  while (!glfwWindowShouldClose(window)) {
    mouseDelta = {0.0f, 0.0f};
    movementInput = { 0.0f, 0.0f };
    float time = (float) glfwGetTime();
    deltaTime = time - lastFrame;
    lastFrame = time;

    keyboardInput(window);
    mouseInput(window);
    camera.quaternionCamera(deltaTime);

    acc += deltaTime;

    if (acc > 1.0f) { // Don't need to flood console
      std::cout << "FPS: " << (int) round(1.0f / deltaTime) << "\n";
      std::cout << "Camera pos: " << camera.position << " Camera view: " << camera.view << "\n";
      acc -= 1.0f;
    }

    // For my system this means mouse forward looks down, and mouse right looks right
    camera.pitch += mouseDelta.y * cameraSpeed * deltaTime;
    camera.yaw += mouseDelta.x * cameraSpeed * deltaTime;
  
    bool canMove = false;
    if (Maths::sqrMagnitude(movementInput) >= 0.01f) {
      canMove = true;
      glm::vec3 playerDir = Maths::normalize(glm::vec3(movementInput.x, 0, movementInput.y));
      for (const BoxCollider2D& collider : colliders) {
        glm::vec3 dir = Maths::normalize(glm::vec3(collider.position.x, 0, collider.position.z) - glm::vec3(camera.position.x, 0, camera.position.z));
        if (BoxCollider2D::isTouching(playerCollider, collider) && Maths::dot(dir, playerDir) < MIN_DOT_PRODUCT) {
          bool canMove = false;
          break;
        }
      }
    }

    if (canMove) {
      glm::vec3 moveDir = camera.forward * movementInput.y + camera.right * movementInput.x;
      moveDir.y = 0;
      camera.position += walkSpeed * deltaTime * Maths::normalize(moveDir);
      playerCollider.updatePosition(camera.position);
    }

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderID);

    glUniform3fv(tintID, 1, glm::value_ptr(camera.tint));
    lights.toShader(shaderID, camera.view);

    for (Object &object : objects) {
      glm::mat4 model = object.modelMat();

      glm::mat4 mv = camera.view * model;
      glm::mat4 mvp = camera.projection * mv;

      glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(mvp));
      glUniformMatrix4fv(mvID, 1, GL_FALSE, glm::value_ptr(mv));

      object.draw(shaderID);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  teapot.deleteBuffers();
  glDeleteProgram(shaderID);
  glfwTerminate();
}

void keyboardInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  movementInput = glm::vec2{0};
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    movementInput.y += 1;

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    movementInput.y -= 1;

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    movementInput.x -= 1;

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    movementInput.x += 1;
}

void mouseInput(GLFWwindow *window) {

  double x = 0, y = 0;
  glfwGetCursorPos(window, &x, &y);
  glfwSetCursorPos(window, width * 0.5f, height * 0.5f);

  mouseDelta = { 0.0f, 0.0f };
  mouseDelta = glm::vec2 { float(x - width * 0.5f), float(height * 0.5f - y) };
}

