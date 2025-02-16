#include "viewport.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <string>

#include "camera.hpp"
#include "scene.hpp"
#include "shader.hpp"

Framebuffer::Framebuffer() {
  glGenFramebuffers(1, &ID);
  glBindFramebuffer(GL_FRAMEBUFFER, ID);

  // use size 10 - will be resized later
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 10, 10, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height) {
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

void Framebuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, ID); }

void Framebuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

Viewport::Viewport(GLFWwindow* window, Scene* scene) : window(window), camera(Camera(1.0, 1.0)), shader(Shader("main")), taaShader(Shader("taa")), scene(scene) {
  createMesh();

  downscaleFactorPrivate = downscaleFactor;

  width = 1;
  height = 1;
  renderWidth = 1;
  renderHeight = 1;

  precalculateHaltonSequence();

  bindKeys();

  // https://discourse.glfw.org/t/what-is-a-possible-use-of-glfwgetwindowuserpointer/1294/2
  glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
  glfwSetScrollCallback(window, inputScrollCallback);
}

Viewport::~Viewport() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void Viewport::resize(int w, int h) {
  if (width == w && height == h && downscaleFactorPrivate == downscaleFactor)
    return;

  downscaleFactorPrivate = downscaleFactor;
  width = w;
  height = h;

  renderWidth = static_cast<int>(static_cast<float>(width) * downscaleFactor);
  renderHeight = static_cast<int>(static_cast<float>(height) * downscaleFactor);

  framebuffer.resize(renderWidth, renderHeight);
  taaFramebuffer.resize(width, height);
  taaHistoryFramebuffer.resize(width, height);
}

void Viewport::render() {
  scene->updateObjectUbo();

  jitterOffset.x = taaFeedbackFactor * haltonSequence[frameCounter % maxFrames].x / static_cast<float>(renderWidth);
  jitterOffset.y = taaFeedbackFactor * haltonSequence[frameCounter % maxFrames].y / static_cast<float>(renderHeight);
  frameCounter++;

  framebuffer.bind();
  glViewport(0, 0, renderWidth, renderHeight);

  shader.use();

  shader.setUniformInt("uRaymarchingSteps", this->raymarchingSteps);
  shader.setUniformFloat("uTime", static_cast<float>(glfwGetTime()));
  shader.setUniformVec2("uResolution", glm::vec2(renderWidth, renderHeight));
  shader.setUniformVec2("uJitterOffset", jitterOffset);
  shader.setUniformVec3("uProj", camera.getProjVec());
  shader.setUniformVec3("uRaymarchingParams", glm::vec3(this->raymarchingClipStart, this->raymarchingClipEnd, this->raymarchingPixelRadius));
  shader.setUniformMat3("uViewRot", camera.getViewRotMat());

  taaShader.setUniformInt("uObjectData", 0);

  glBindVertexArray(VAO);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  taaFramebuffer.bind();
  glViewport(0, 0, width, height);

  taaShader.use();

  taaShader.setUniformFloat("uFeedbackFactor", taaFeedbackFactor);
  taaShader.setUniformVec2("uJitterOffset", jitterOffset);
  taaShader.setUniformVec2("uResolution", glm::vec2(width, height));

  taaShader.setUniformInt("uCurrentFrame", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, framebuffer.textureID);

  taaShader.setUniformInt("uHistoryFrame", 1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, taaHistoryFramebuffer.textureID);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, taaFramebuffer.ID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, taaHistoryFramebuffer.ID);
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::createMesh() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // using a single triangle is more efficient [https://stackoverflow.com/a/59739538]
  float vertices[] = {
      -1.0f, -1.0f, 0.0f, // bottom left
      3.0f,  -1.0f, 0.0f, // bottom right
      -1.0f, 3.0f,  0.0f  // top left
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindVertexArray(0);
}

void Viewport::precalculateHaltonSequence() {
  for (int i = 0; i < maxFrames; ++i) {
    // +1 to avoid 0, -0.5 to bring to (-0.5,0.5) range
    haltonSequence[i].x = halton(i + 1, 2) - 0.5f;
    haltonSequence[i].y = halton(i + 1, 3) - 0.5f;
  }
}

float Viewport::halton(int index, int base) const {
  float result = 0.0f;
  float baseinv = 1.0f / static_cast<float>(base);
  float f = baseinv;
  int i = index;
  while (i > 0) {
    result += f * static_cast<float>(i % base);
    i = i / base;
    f = f * baseinv;
  }
  return result;
}

void Viewport::bindKeys() {
  // TODO: might have to use mods in future for more combinations
  static std::map<int, std::function<void()>> keyBinds;
  keyBinds[GLFW_KEY_KP_1] = [&]() { camera.setViewpoint(-1, 0); };
  keyBinds[GLFW_KEY_KP_3] = [&]() { camera.setViewpoint(0, 0); };
  keyBinds[GLFW_KEY_KP_7] = [&]() { camera.setViewpoint(0, 1); };
  keyBinds[GLFW_KEY_KP_9] = [&]() { camera.setViewpoint(0, -1); };
  keyBinds[GLFW_KEY_KP_5] = [&]() { camera.toggleOrthoView(); };
  keyBinds[GLFW_KEY_R] = [&]() { shader.reloadFragment(); }; // TODO: use ctrl mod

  // TODO: must only do when cursor is inside window
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
      if (keyBinds.count(key)) {
        keyBinds[key]();
      }
    }
  });
}

void Viewport::inputScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  Viewport* vp = reinterpret_cast<Viewport*>(glfwGetWindowUserPointer(window));
  if (!vp->hovered)
    return;

  float x = static_cast<float>(xoffset);
  float y = static_cast<float>(yoffset);

  x *= vp->cameraSensitivity * x * glm::sign(x);
  y *= vp->cameraSensitivity * y * glm::sign(y);

  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    vp->camera.dist = glm::max(vp->camera.dist + x - y, 0.1f);
  } else {
    vp->camera.yaw += x;
    vp->camera.pitch += y;
  }
}
