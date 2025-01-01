#include "scene.hpp"

#include <map>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

void Scene::addObject() {
  sceneTree[0] = {0, 0, glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0), glm::vec3(1.0), glm::vec4(1.0)};
  sceneTree[1] = {0, 0, glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0), glm::vec3(1.0), glm::vec4(1.0)};
  sceneTree[2] = {0, 0, glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0), glm::vec3(1.0), glm::vec4(1.0)};
  sceneTree[3] = {0, 0, glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0), glm::vec3(1.0), glm::vec4(1.0)};
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
    ObjectUboData data;
    glm::mat4 model = glm::mat4(1.0f);
    // model = glm::scale(model, pair.second.scale);
    //  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    //  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    //  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, pair.second.position);
    data.model = model;
    // data.matId = pair.second.matId;
    objectData.push_back(data);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, objectUbo);

  // member offset size
  // int    0      4
  // -      -      12 <- padding (required to keep offsets be multiple of 16
  // mat4   16     64
  unsigned long objectsNum = objectData.size();
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &objectsNum);
  glBufferSubData(GL_UNIFORM_BUFFER, 16, static_cast<long>(objectsNum * sizeof(ObjectUboData)), objectData.data());

  glBindBufferRange(GL_UNIFORM_BUFFER, 0, objectUbo, 0, static_cast<long>(16 + (objectsNum * sizeof(ObjectUboData)))); // Important: Bind with correct size

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
