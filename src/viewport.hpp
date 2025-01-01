#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <array>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.hpp"
#include "scene.hpp"
#include "shader.hpp"

// TODO: Move this somewhere else
class Framebuffer {
public:
  unsigned int ID, textureID, RID;

  Framebuffer();

  void resize(int width, int height);

  void bind() const;

  void unbind() const;
};

class Viewport {
public:
  GLuint VAO, VBO;

  Shader shader;
  Shader taaShader;

  Framebuffer framebuffer;
  Framebuffer taaFramebuffer;
  Framebuffer taaHistoryFramebuffer;

  Scene* scene;

  Camera camera;

  GLFWwindow* window;

  int width, height;
  int renderWidth, renderHeight;
  float downscaleFactor = 0.2f;

  float cameraSensitivity = 0.2f;

  glm::vec2 jitterOffset, previousJitterOffset;
  float taaFeedbackFactor = 0.95f;
  int frameCounter = 0;

  int raymarchingSteps = 32;
  float raymarchingClipStart = 0.0;
  float raymarchingClipEnd = 10.0;
  float raymarchingHitBias = 0.01;

  static constexpr int maxFrames = 128;
  std::array<glm::vec2, maxFrames> haltonSequence;

  GLuint ubo;

  Viewport(GLFWwindow* window, Scene* scene);

  ~Viewport();

  void resize(int w, int h);

  void render();

  static void inputScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
  float downscaleFactorPrivate;

  void createMesh();

  void precalculateHaltonSequence();

  float halton(int index, int base) const;

  void bindKeys();
};

#endif
