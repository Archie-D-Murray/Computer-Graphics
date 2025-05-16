#include "glm/detail/type_vec.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include <set>
#include FT_FREETYPE_H

#include <common/box_collider2d.hpp>
#include <common/camera.hpp>
#include <common/light.hpp>
#include <common/maths.hpp>
#include <common/model.hpp>
#include <common/object.hpp>
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <string>

glm::vec2 mouseDelta;
glm::vec2 movementInput;

float lastFrame = 0.0f;
float deltaTime = 0.0f;
float cameraTimer = 0.0f;
float renderTimer = 0.0f;
float areaTimer = 0.0f;

enum Interaction { NONE, SPEED_INCREMENT, SPEED_DECREMENT, DIFFUSE };

bool canInteract = false;
float objectSpeed = 0.5f;
float objectIncrement = 0.125f;
float maxSpeed = 1.0f;
float minSpeed = 0.125f;
float objectProgress = 0.0f;
float targetZ = -3.0f;
int objectIndex = 0;
glm::vec4 objectDiffuseSettings = glm::vec4(0.7f, 0.7f, 1.0f, 20.0f);
glm::vec4 diffuseInput = glm::vec4(0);
glm::vec4 minSettings = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
glm::vec4 maxSettings = glm::vec4(1.0f, 1.0f, 1.0f, 20.0f);
Interaction interaction = NONE;

const float width = 1260;
const float height = 720;
const float cameraSpeed = 0.075f;
const float walkSpeed = 1.0f;
const float cameraDelay = 0.25f;

enum CameraType { FPS, NE, SE, SW, NW, CAMERA_COUNT };
Camera cameras[CAMERA_COUNT] = {
    Camera({+0.0f, +1.0f, +3.0f}, {0.0f, 0.0f, 0.0f}),
    Camera({+5.0f, +4.0f, +5.0f}, {0.0f, 0.0f, 0.0f}),
    Camera({+5.0f, +4.0f, -5.0f}, {0.0f, 0.0f, 0.0f}),
    Camera({-5.0f, +4.0f, +5.0f}, {0.0f, 0.0f, 0.0f}),
    Camera({-5.0f, +4.0f, -5.0f}, {0.0f, 0.0f, 0.0f}),
};
CameraType camera = FPS;

struct Character {
  uint32_t textureID;
  glm::ivec2 size;    // Glyph Size
  glm::ivec2 bearing; // Centre Pos
  uint32_t next;
};

Character characters[128];

struct TextRenderData {
  std::string text;
  glm::ivec2 position;
  float scale;
  glm::vec3 colour;
};

std::vector<TextRenderData> textQueue;
bool collisionDebugRendering = false;

// Function prototypes
void keyboardInput(GLFWwindow *window);
void mouseInput(GLFWwindow *window);
inline Camera &currentCamera() { return cameras[camera]; }

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
  window = glfwCreateWindow((int)width, (int)height, "Computer Graphics", NULL,
                            NULL);

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

  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    std::cout << "FreeType Error: Could not init\n";
    return -1;
  }

  FT_Face font;
  if (FT_New_Face(ft, "../assets/jetbrains_mono_regular.ttf", 0, &font)) {
    std::cout << "FreeType Error: Could not load font\n";
    return -1;
  }

  FT_Set_Pixel_Sizes(font, 0, 48);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (size_t i = 0; i < 128; i++) {
    if (FT_Load_Char(font, (FT_ULong) i, FT_LOAD_RENDER)) {
      std::cout << "Could not load character: " << (char)i << "\n";
      continue;
    }
    Character *character = &characters[i];
    glGenTextures(1, &character->textureID);
    glBindTexture(GL_TEXTURE_2D, character->textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->glyph->bitmap.width,
                 font->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                 font->glyph->bitmap.buffer);
    // No repeat + Linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    character->size =
        glm::ivec2(font->glyph->bitmap.width, font->glyph->bitmap.rows);
    character->bearing =
        glm::ivec2(font->glyph->bitmap_left, font->glyph->bitmap_top);
    character->next = font->glyph->advance.x;
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  // Free FreeType internal memory
  FT_Done_Face(font);
  FT_Done_FreeType(ft);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 0);

  // Init Text Buffers
  uint32_t textShaderID =
      LoadShaders("./textVertexShader.glsl", "./textFragmentShader.glsl");
  uint32_t textVAO, textVBO;
  glGenVertexArrays(1, &textVAO);
  glGenBuffers(1, &textVBO);
  glBindVertexArray(textVAO);
  glBindBuffer(GL_ARRAY_BUFFER, textVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr,
               GL_DYNAMIC_DRAW); // One Quad = 6 vertices = 6 * 4 floats
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);

  // Use back face culling
  glEnable(GL_CULL_FACE);

  // Ensure we can capture keyboard inputs
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Capture mouse inputs
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwPollEvents();
  glfwSetCursorPos(window, width * 0.5f, height * 0.5f);

  uint32_t shaderID =
      LoadShaders("./vertexShader.glsl", "./fragmentShader.glsl");
  uint32_t mvpID = glGetUniformLocation(shaderID, "MVP");
  uint32_t mvID = glGetUniformLocation(shaderID, "MV");
  uint32_t tintID = glGetUniformLocation(shaderID, "tint");
  glUniform3fv(glGetUniformLocation(shaderID, "modelTint"), 1, glm::value_ptr(glm::vec3(1.0f)));

  Light lights;

  lights.addDirectionalLight(glm::vec3(1.0, -1.0f, 0.0f), glm::vec3(0.8f, 1.0f, 0.8f));
  lights.addSpotLight(glm::vec3{0, 3, 0}, glm::vec3{0.0f, -1, 0}, glm::vec3(0.8f, 0.8f, 1.0f), 1.0f, 0.1f, 0.02f, Maths::radians(45));
  lights.addPointLight(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.1f, 0.5f, 0.5f), 1.0f, 0.1f, 0.02f);

  std::vector<BoxCollider2D> colliders;
  std::vector<Object> objects;

  float hue = 0.1f;

  BoxCollider2D playerCollider = BoxCollider2D(currentCamera().position, glm::vec2(0.5f));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  Model tux = Model("../assets/tux.obj");
  tux.addTexture("../assets/onyx_diffuse.png", "diffuse");
  tux.addTexture("../assets/onyx_normal.png", "normal");
  tux.addTexture("../assets/onyx_specular.png", "specular");
  tux.setDiffusionParameters(0.7f, 0.7f, 1.0f, 20.0f);

  Model box = Model("../assets/crate.obj");
  box.addTexture("../assets/metal_diffuse.png", "diffuse");
  box.addTexture("../assets/metal_normal.png", "normal");
  box.addTexture("../assets/metal_specular.png", "specular");
  box.setDiffusionParameters(0.5f, 0.5f, 0.5f, 20.0f);

  Model wall = Model("../assets/small_plane.obj");
  wall.addTexture("../assets/bricks_diffuse.png", "diffuse");
  wall.addTexture("../assets/bricks_normal.png", "normal");
  wall.addTexture("../assets/bricks_specular.png", "specular");
  wall.setDiffusionParameters(0.1f, 0.7f, 1.0f, 20.0f);

  Model floor = Model("../assets/small_plane.obj");
  floor.addTexture("../assets/wood_diffuse.png", "diffuse");
  floor.addTexture("../assets/wood_normal.png", "normal");
  floor.addTexture("../assets/wood_specular.png", "specular");
  floor.setDiffusionParameters(0.05f, 0.7f, 1.0f, 20.0f);

  Model ceiling = Model("../assets/small_plane.obj");
  ceiling.addTexture("../assets/plaster_diffuse.png", "diffuse");
  ceiling.addTexture("../assets/plaster_normal.png", "normal");
  ceiling.addTexture("../assets/plaster_specular.png", "specular");
  ceiling.setDiffusionParameters(0.2f, 0.7f, 1.0f, 20.0f);

  Model teapot = Model("../assets/teapot.obj");
  teapot.addTexture("../assets/white.png", "diffuse");
  teapot.addTexture("../assets/white.png", "normal");
  teapot.addTexture("../assets/white.png", "specular");

  Model marble = Model("../assets/teapot.obj");
  marble.addTexture("../assets/marble_diffuse.png", "diffuse");
  marble.addTexture("../assets/marble_normal.png", "normal");
  marble.addTexture("../assets/marble_specular.png", "specular");
  marble.setDiffusionParameters(objectDiffuseSettings);

  Model colliderDebug = Model("../assets/unit_cube.obj");
  colliderDebug.addTexture("../assets/white.png", "diffuse");
  colliderDebug.addTexture("../assets/white.png", "normal");
  colliderDebug.addTexture("../assets/white.png", "specular");

  fprintf(stdout, "Loaded models\n");

  objects.push_back(Object(glm::vec3{0, 0, 0}, glm::vec3(0.01f),
                           Quaternion(), // Identity quaternion
                           "Tux", &tux));

  objects.push_back(Object(glm::vec3{0, 0.0f, 0.f},
                           glm::vec3(0.75f), Quaternion(), "Box",
                           &box));

  objects.push_back(Object(glm::vec3(5.0f, 2.5f, 0.0f), glm::vec3(2.0f),
                           Quaternion(0.5f * M_PI, -0.5f * M_PI), "East",
                           &wall));

  objects.push_back(Object(glm::vec3(-5.0f, 2.5f, 0.0f), glm::vec3(2.0f),
                           Quaternion(0.5f * M_PI, 0.5f * M_PI), "West",
                           &wall));

  objects.push_back(Object(glm::vec3(0.0f, 2.5f, -5.0f), glm::vec3(2.0f),
                           Quaternion(0.5f * M_PI, 0), "North", &wall));

  objects.push_back(Object(glm::vec3(0.0f, 2.5f, 5.0f), glm::vec3(2.0f),
                           Quaternion(0.5f * M_PI, M_PI), "South", &wall));

  objects.push_back(Object(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f),
                           Quaternion(0, 0), "Floor", &floor));

  objects.push_back(Object(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(2.0f),
                           Quaternion(M_PI, 0), "Ceiling", &ceiling));

  objects.push_back(Object(glm::vec3(4, 0, 3), glm::vec3(1.0f), Quaternion(), "Object", &marble));
  Object* object = &objects.back();

  colliders.push_back(BoxCollider2D({0, 0, 0}, {1.0f, 1.0f})); // Centre Crate
  colliders.push_back(BoxCollider2D({0, 0, +5.5f}, {12, 1}));  // South
  colliders.push_back(BoxCollider2D({0, 0, -5.5f}, {12, 1}));  // North
  colliders.push_back(BoxCollider2D({+5.5f, 0, 0}, {1, 12}));  // Right
  colliders.push_back(BoxCollider2D({-5.5f, 0, 0}, {1, 12}));  // Left

  cameras[FPS].lookAt(objects[0].position);
  cameras[NE].lookAt(glm::vec3(0.0f));
  cameras[SE].lookAt(glm::vec3(0.0f));
  cameras[SW].lookAt(glm::vec3(0.0f));
  cameras[NW].lookAt(glm::vec3(0.0f));
  cameras[NW].lookAt(glm::vec3(0.0f));
  cameras[NE].tint = glm::vec3(1.0f, 0.5f, 0.5f);
  cameras[SE].tint = glm::vec3(0.5f, 1.0f, 0.5f);
  cameras[SW].tint = glm::vec3(0.5f, 0.5f, 1.0f);
  cameras[NW].tint = glm::vec3(1.0f, 1.0f, 0.0f);

  float acc = 0;

  textQueue.push_back(TextRenderData{ std::string("FPS: 0"), glm::vec2(10, 670), 1.0f, glm::vec3(1.0f, 1.0f, 0.0f) });

  while (!glfwWindowShouldClose(window)) {
    mouseDelta = {0.0f, 0.0f};
    movementInput = {0.0f, 0.0f};
    float time = (float)glfwGetTime();
    deltaTime = time - lastFrame;
    lastFrame = time;
    if (cameraTimer > 0) {
      cameraTimer -= deltaTime;
    }
    if (renderTimer > 0) {
      renderTimer -= deltaTime;
    }
    if (areaTimer > 0) {
      areaTimer -= deltaTime;
    }

    hue += deltaTime;
    if (hue > 1.1f) {
      hue -= 1.0f;
    }

    keyboardInput(window);
    mouseInput(window);
    currentCamera().quaternionCamera(deltaTime);

    acc += deltaTime;

    if (acc > 1.0f) { // Don't need to flood console
      const int bufLen = 32;
      char buf[bufLen];
      sprintf(buf, "FPS: %d", int(round(1.0f / deltaTime)));
      textQueue.front().text = std::string(buf);
      // std::cout << "Player: " << playerCollider << "\n";
      // std::cout << "Teapot: " << colliders.front() << "\n";
      acc -= 1.0f;
      memset(buf, 0, bufLen * sizeof(char));
    }

    // For my system this means mouse forward looks down, and mouse right looks
    // right
    if (camera == FPS) {
      currentCamera().pitch += mouseDelta.y * cameraSpeed * deltaTime;
      currentCamera().yaw += mouseDelta.x * cameraSpeed * deltaTime;
    }

    bool canMove = false;
    if (Maths::sqrMagnitude(movementInput) >= 0.01f) {
      canMove = true;
      glm::vec3 moveDir3 = currentCamera().forward * movementInput.y +
                           currentCamera().right * movementInput.x;
      glm::vec2 moveDir = Maths::normalize(glm::vec2(moveDir3.x, moveDir3.z));
      glm::vec2 playerPos =
          glm::vec2(currentCamera().position.x, currentCamera().position.z);
      for (const BoxCollider2D &collider : colliders) {
        glm::vec2 obstacleDir =
            Maths::normalize(collider.getClosestPoint(playerPos) - playerPos);
        if (BoxCollider2D::isTouching(playerCollider, collider)) {
          if (Maths::dot(obstacleDir, moveDir) > MIN_DOT_PRODUCT) {
            canMove = false;
            break;
          }
        }
      }
    }

    if (canMove && camera == FPS) {
      glm::vec3 moveDir = currentCamera().forward * movementInput.y +
                          currentCamera().right * movementInput.x;
      moveDir.y = 0;
      currentCamera().position +=
          walkSpeed * deltaTime * Maths::normalize(moveDir);
      playerCollider.updatePosition(currentCamera().position);
    }

    const BoxCollider2D inputArea = BoxCollider2D(glm::vec3(3, 0, 0), glm::vec2(4, 10));
    canInteract = BoxCollider2D::isTouching(playerCollider, inputArea);
    if (canInteract) {
      char buf[72];
      sprintf(buf, "Speed %.2fx, Diffuse Settings: ka, kd, ks, Ns: [ %.1f, %.1f, %.1f, %.1f ]", objectSpeed / 0.5f, objectDiffuseSettings.x, objectDiffuseSettings.y, objectDiffuseSettings.z, objectDiffuseSettings.w); textQueue.push_back(TextRenderData {std::string(buf), glm::ivec2(10, 500), 0.33f, glm::vec3(1.0f)});
    }
    if (canInteract && interaction != NONE) {
      switch (interaction) {
        case NONE:
          assert("UNREACHABLE");
          break;
        case SPEED_INCREMENT:
          objectSpeed = fminf(maxSpeed, objectSpeed + objectIncrement);
          break;
        case SPEED_DECREMENT:
          objectSpeed = fmaxf(minSpeed, objectSpeed - objectIncrement);
          break;
        case DIFFUSE:
          objectDiffuseSettings = Maths::clamp(minSettings, maxSettings, objectDiffuseSettings + deltaTime * diffuseInput);
          marble.setDiffusionParameters(objectDiffuseSettings);
          break;
      }
      interaction = NONE;
    } else if (interaction != NONE) {
      interaction = NONE; 
    }

    objectProgress = fminf(1.0f, objectProgress + deltaTime * objectSpeed);
    if (objectProgress >= 1.0f) {
      objectProgress = 0.0f;
      targetZ *= -1.0f;
    }

    object->position = glm::vec3(object->position.x, 1.0f + 0.5f * sinf(time * M_PI * 0.25f), Maths::lerp(-targetZ, targetZ, Maths::smoothDamp(objectProgress)));
    object->rotation = Quaternion(0, time * M_PI * 0.125f);

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderID);

    glUniform3fv(tintID, 1, glm::value_ptr(currentCamera().tint));
    lights.toShader(shaderID, currentCamera().view);

    objects.front().tint = Maths::hslToRGB(glm::vec3(hue, 1, 0.75f));
    objects.front().rotation = Quaternion(0, time * M_PI);
    objects.front().position =
        glm::vec3(0, sinf(time * M_PI) * 0.25f + 1.0f, 0);

    for (Object &object : objects) {
      glm::mat4 model = object.modelMat();

      glm::mat4 mv = currentCamera().view * model;
      glm::mat4 mvp = currentCamera().projection * mv;

      glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(mvp));
      glUniformMatrix4fv(mvID, 1, GL_FALSE, glm::value_ptr(mv));

      object.draw(shaderID);
    }

    if (collisionDebugRendering) {
      for (BoxCollider2D &collider : colliders) {
        glm::mat4 model = Maths::translate(collider.position) * Maths::scale(glm::vec3(collider.size.x, 1, collider.size.y) * 1.05f);
        glm::mat4 mv = currentCamera().view * model;
        glm::mat4 mvp = currentCamera().projection * mv;

        glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(mvID, 1, GL_FALSE, glm::value_ptr(mv));

        colliderDebug.draw(shaderID);
      }
    }

    if (camera != FPS) {
        glm::mat4 model = Maths::translate(cameras[FPS].position) * Quaternion(0.0f, 0.5f * M_PI - cameras[FPS].yaw).matrix() * Maths::scale(glm::vec3(1.0f));
        glm::mat4 mv = currentCamera().view * model;
        glm::mat4 mvp = currentCamera().projection * mv;

        glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(mvID, 1, GL_FALSE, glm::value_ptr(mv));

        teapot.draw(shaderID);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const glm::mat4 textProjection =
        Maths::ortho(0.0f, width, 0.0f, height, 0.0f, 10.0f);
    glUseProgram(textShaderID);
    glUniformMatrix4fv(glGetUniformLocation(textShaderID, "projection"), 1,
                       GL_FALSE, glm::value_ptr(textProjection));
    for (TextRenderData &data : textQueue) {
      float x = data.position.x;
      float y = data.position.y;
      glUniform3fv(glGetUniformLocation(textShaderID, "textColour"), 1,
                   glm::value_ptr(data.colour));
      glActiveTexture(GL_TEXTURE0);
      glBindVertexArray(textVAO);
      for (std::string::iterator it = data.text.begin(); it != data.text.end();
           it++) {
        Character *ch = &characters[*it];
        if (*it == '\n') {
          x = data.position.x;
          y -= (ch->next >> 5) * data.scale;
          continue;
        }
        glm::vec2 pos =
            glm::vec2(x + ch->bearing.x * data.scale,
                      y - (ch->size.y - ch->bearing.y) * data.scale);
        glm::vec2 size = data.scale * glm::vec2(ch->size);

        float vertices[6][4] = {
            {pos.x, pos.y + size.y, 0.0f, 0.0f},
            {pos.x, pos.y, 0.0f, 1.0f},
            {pos.x + size.x, pos.y, 1.0f, 1.0f},
            {pos.x, pos.y + size.y, 0.0f, 0.0f},
            {pos.x + size.x, pos.y, 1.0f, 1.0f},
            {pos.x + size.x, pos.y + size.y, 1.0f, 0.0f},
        };
        glBindTexture(GL_TEXTURE_2D, ch->textureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch->next >> 6) * data.scale;
      }
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    while (textQueue.size() > 1) { // Always want FPS counter
      textQueue.pop_back();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  std::set<Model*> models;
  for (Object& object : objects) {
    models.insert(object.model);
  }
  for (Model* model : models) {
    model->deleteBuffers();
  }
  for (Character& ch : characters) {
    glDeleteTextures(1, &ch.textureID);
  }
  glDeleteBuffers(1, &textVAO);
  glDeleteBuffers(1, &textVBO);
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

  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && cameraTimer <= 0.0f) {
    std::cout << "Camera: " << camera << "\n";
    cameraTimer += cameraDelay;
    camera = (CameraType)((camera + 1) % CAMERA_COUNT);
  }

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && renderTimer <= 0.0f) {
    collisionDebugRendering = !collisionDebugRendering;
    renderTimer += 1.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && cameraTimer <= 0.0f) {
    cameraTimer += cameraDelay;
    int newCamera = camera - 1;
    if (newCamera < 0) {
      newCamera = CAMERA_COUNT - 1;
    }
    camera = (CameraType)newCamera;
  }

  if (!canInteract || areaTimer > 0.0f) { return; }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    areaTimer += 1.0f;
    interaction = SPEED_INCREMENT;
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    areaTimer += 1.0f;
    interaction = SPEED_DECREMENT;
  }

  diffuseInput.x = int(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS);
  diffuseInput.y = int(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS);
  diffuseInput.z = int(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS);
  diffuseInput.w = int(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) - int(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);
  
  if (Maths::sqrMagnitude(diffuseInput) > 0) {
    interaction = DIFFUSE;
  }
}

void mouseInput(GLFWwindow *window) {

  double x = 0, y = 0;
  glfwGetCursorPos(window, &x, &y);
  glfwSetCursorPos(window, width * 0.5f, height * 0.5f);

  mouseDelta = {0.0f, 0.0f};
  mouseDelta = glm::vec2{float(x - width * 0.5f), float(height * 0.5f - y)};
}
