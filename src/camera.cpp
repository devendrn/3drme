#include "camera.hpp"

#include <glm/common.hpp>
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

void Camera::displaceTarget(glm::vec2 delta) {
  float sy = glm::sin(yaw);
  float cy = glm::cos(yaw);
  glm::vec3 hoffset = glm::vec3(cy, 0.0, sy);
  glm::vec3 voffset;
  voffset.y = glm::cos(pitch);
  voffset.z = glm::sin(pitch);
  voffset.x = -sy * voffset.z;
  voffset.z = cy * voffset.z;

  target += 1.5f * hoffset * delta.x;
  target -= 1.5f * voffset * delta.y;
}

void Camera::offsetYawPitch(float yawDelta, float pitchDelta) {
  yaw = glm::mod(yaw + yawDelta, glm::two_pi<float>());
  pitch = glm::mod(pitch + pitchDelta, glm::two_pi<float>());
}
