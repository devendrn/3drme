#ifndef SCENE_H
#define SCENE_H

#include <map>

#include <glad/glad.h>

#include <glm/glm.hpp>

struct Object {
  int type;
  int matId;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::vec4 extra;
};

struct Material {
  glm::vec3 color;
};

struct Light {
  glm::vec3 color;
  glm::vec3 position;
};

struct MaterialUboData {
  glm::vec4 color;
};

struct ObjectUboData {
  // int type;
  // int matId;
  glm::mat4 model;
};

/*
struct LightUBO {
  int type;
  glm::mat4 transform;
};
*/

class Scene {
public:
  std::map<unsigned int, Object> sceneTree;   // id, obj
  std::map<unsigned int, Material> materials; // id, mat

  GLuint objectUbo;
  GLuint materialUbo;

  Scene();

  void addObject();

  void modifyObjectPosition(unsigned int id, glm::vec3 position);

  void deleteObject();

  void updateObjectUbo();
};

#endif
