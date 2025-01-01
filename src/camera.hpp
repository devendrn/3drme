#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
  float dist, yaw, pitch, fov, scale;
  glm::vec3 target;
  bool isOrtho;

  Camera(float fov, float scale);

  glm::vec3 getProjVec() const;

  glm::mat3 getViewRotMat() const;

  void setViewpoint(int yawi, int pitchi);

  void toggleOrthoView();
};

#endif
