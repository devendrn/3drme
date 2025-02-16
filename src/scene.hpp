#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>

struct Object {
  unsigned int id;
  std::string name;
  int type;
  int matId = 0;
  int mode = 0; // right now only used as boolean for selected
  glm::vec3 position = glm::vec3(0.0f);
  glm::vec3 rotation = glm::vec3(0.0f);
  glm::vec3 scale = glm::vec3(1.0f);
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
  glm::mat4 transformation; // not a composite matrix
  glm::ivec4 typeMatIdMode; // x=type, y=mat_id, z=mode, w is for padding
};

/*
struct LightUBO {
  int type;
  glm::mat4 transform;
};
*/

enum Shape {
  BOX = 0,
  SPHERE = 1,
};

class Scene {
public:
  std::vector<Object> sceneTree;
  std::map<unsigned int, Material> materials; // id, mat

  GLuint objectUbo;
  GLuint materialUbo;

  Scene();

  void addObject(Shape shape);

  void modifyObjectPosition(unsigned int id, glm::vec3 position);

  void deleteObject(unsigned int index);

  void updateObjectUbo();

  void selectObject(unsigned int index);

  void deselectObjects();

private:
  int lastSelectedObjectIndex = -1;

  glm::mat4 constructTransformationMat(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation);
};

#endif
