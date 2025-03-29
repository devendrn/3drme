#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <array>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
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

  bool hovered;

  int width, height;
  int renderWidth, renderHeight;
  float downscaleFactor = 0.5f;

  float cameraSensitivity = 0.2f;

  glm::vec2 jitterOffset, previousJitterOffset;
  float taaFeedbackFactor = 0.92f;
  int frameCounter = 0;

  int raymarchingSteps = 32;
  float raymarchingClipStart = 0.5f;
  float raymarchingClipEnd = 20.0f;
  float raymarchingPixelRadius = 0.002f;

  glm::vec3 ambientColor = glm::vec3(1.0);
  float occlusionFactor = 1.0;
  float occlusionRadius = 0.2;

  static constexpr int maxFrames = 128;
  std::array<glm::vec2, maxFrames> haltonSequence;

  GLuint ubo;

  Viewport(GLFWwindow* window, Scene* scene);

  ~Viewport();

  void resize(int w, int h);

  void render();

  static void inputScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

  void captureImage(std::string& file) const;

private:
  float downscaleFactorPrivate;

  void createMesh();

  void precalculateHaltonSequence();

  float halton(int index, int base) const;

  void bindKeys();
};

#endif
