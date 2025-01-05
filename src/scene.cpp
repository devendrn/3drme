#include "scene.hpp"

#include <map>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

void Scene::addObject() {
  // TEST!!
  sceneTree[0] = {1, 0, glm::vec3(2.0, 0.0, 0.0), glm::vec3(0.0), glm::vec3(1.0), glm::vec4(1.0)};
  sceneTree[1] = {0, 0, glm::vec3(0.0, 0.0, 2.0), glm::vec3(0.5, 0.0, 0.0), glm::vec3(1.0), glm::vec4(1.0)};
  sceneTree[2] = {1, 0, glm::vec3(0.0, -3.0, 0.0), glm::vec3(0.0), glm::vec3(1.5), glm::vec4(1.0)};
  sceneTree[3] = {0, 0, glm::vec3(-3.0, 0.0, 0.0), glm::vec3(0.0, -0.7, 0.0), glm::vec3(0.8), glm::vec4(1.0)};
}

Scene::Scene() {
  glGenBuffers(1, &objectUbo);
  glBindBuffer(GL_UNIFORM_BUFFER, objectUbo);
  glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(ObjectUboData), nullptr, GL_DYNAMIC_DRAW); // assuming scene contains at most 32 objects
  // glBindBufferBase(GL_UNIFORM_BUFFER, 0, objectUbo);                                     // bind location = 0
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  /*
  glGenBuffers(1, &materialUbo);
  glBindBuffer(GL_UNIFORM_BUFFER, materialUbo);
  glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(MaterialUboData), nullptr, GL_DYNAMIC_DRAW); // assuming scene contains at most 32 mats
  glBindBufferBase(GL_UNIFORM_BUFFER, 2, materialUbo);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  */
}

void Scene::updateObjectUbo() {
  std::vector<ObjectUboData> objectData;

  for (const auto& pair : sceneTree) {
    const Object& v = pair.second;
    ObjectUboData data;
    data.transformation = constructTransformationMat(v.position, v.scale, v.rotation);
    data.typeMatId = glm::ivec4(v.type, v.matId, 0, 0);
    objectData.push_back(data);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, objectUbo);

  // member offset size
  // int    0      4
  // -      -      12 <- padding (required to keep offsets be multiple of 16
  // obj    16     -
  unsigned long objectsNum = objectData.size();
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &objectsNum);
  glBufferSubData(GL_UNIFORM_BUFFER, 16, static_cast<long>(objectsNum * sizeof(ObjectUboData)), objectData.data());

  glBindBufferRange(GL_UNIFORM_BUFFER, 0, objectUbo, 0, static_cast<long>(16 + (objectsNum * sizeof(ObjectUboData))));

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
/*
     void updateMaterialUBO() {
         std::vector<MaterialData> materialData;
         for (const auto& pair : materials) {
             MaterialData data;
             data.color = pair.second.color;
             materialData.push_back(data);
         }
         glBindBuffer(GL_UNIFORM_BUFFER, materialUBO);
         glBufferSubData(GL_UNIFORM_BUFFER, 0, materialData.size() * sizeof(MaterialData), materialData.data());
         glBindBuffer(GL_UNIFORM_BUFFER, 0);
     }
*/

void Scene::modifyObjectPosition(unsigned int id, glm::vec3 position) { sceneTree[id].position = position; }

glm::mat4 Scene::constructTransformationMat(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) {
  glm::vec3 s = glm::sin(rotation);
  glm::vec3 c = glm::cos(rotation);

  // TODO: Use quaternion?
  return glm::mat4(                                                                    // Packing in this manner to save space and reduce calculations in shader
      c.y * c.z, s.x * s.y * c.z - c.x * s.z, c.x * s.y * c.z + s.x * s.z, -position.x, //
      c.y * s.z, s.x * s.y * s.z + c.x * c.z, c.x * s.y * s.z - s.x * c.z, -position.y, //
      -s.y, s.x * c.y, c.x * c.y, -position.z,                                          //
      1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z, 1.0);                            //
}
