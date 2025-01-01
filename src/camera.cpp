#include "camera.hpp"

#include <glm/gtc/constants.hpp>

Camera::Camera(float fov, float scale) : dist(3.0), yaw(0.78), pitch(0.39), fov(fov), scale(scale), isOrtho(false) {}

glm::vec3 Camera::getProjVec() const {
  if (isOrtho)
    return glm::vec3(0.5 + dist, 0.0, 10.0);

  return glm::vec3(scale, fov, dist);
}

glm::mat3 Camera::getViewRotMat() const {
  float sy = glm::sin(yaw);
  float sp = glm::sin(pitch);
  float cy = glm::cos(yaw);
  float cp = glm::cos(pitch);
  return glm::mat3(cy, -sp * sy, -sy * cp, 0.0, cp, -sp, sy, sp * cy, cy * cp);
}

void Camera::setViewpoint(int yawi, int pitchi) {
  yaw = static_cast<float>(yawi) * glm::half_pi<float>();
  pitch = static_cast<float>(pitchi) * glm::half_pi<float>();
}

void Camera::toggleOrthoView() { isOrtho = !isOrtho; }
