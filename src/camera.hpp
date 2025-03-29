#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
  float dist, yaw, pitch, fov, scale;
  glm::vec3 target = glm::vec3(0.0);
  bool isOrtho;

  Camera(float fov, float scale);

  glm::vec3 getProjVec() const;
  glm::mat3 getViewRotMat() const;
  void displaceTarget(glm::vec2 delta);
  void setViewpoint(int yawi, int pitchi);
  void toggleOrthoView();
  void offsetYawPitch(float yawDelta, float pitchDelta);
};

#endif
