#include <fstream>
#include <iostream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <glm/fwd.hpp>

#include "camera.hpp"
#include "projectdata.hpp"
#include "scene.hpp"
#include "viewport.hpp"

namespace cereal {

template <class Archive> void serialize(Archive& archive, glm::vec3& d) { archive(d.x, d.y, d.z); };
template <class Archive> void serialize(Archive& archive, glm::vec4& d) { archive(d.x, d.y, d.z, d.w); };
template <class Archive> void serialize(Archive& archive, Object& d) { archive(d.id, d.name, d.type, d.matId, d.mode, d.position, d.rotation, d.scale, d.extra); };
template <class Archive> void serialize(Archive& archive, Material& d) { archive(d.color); }
template <class Archive> void serialize(Archive& archive, Scene& d) { archive(d.sceneTree, d.materials); }
template <class Archive> void serialize(Archive& archive, Camera& d) { archive(d.dist, d.yaw, d.pitch, d.fov, d.scale, d.target, d.isOrtho); }
template <class Archive> void serialize(Archive& archive, Viewport& d) { archive(d.camera); }

} // namespace cereal

ProjectData::ProjectData() { nodeEditorContext = ax::NodeEditor::CreateEditor(); };

ProjectData::~ProjectData() { ax::NodeEditor::DestroyEditor(nodeEditorContext); }

void ProjectData::saveProjectFile(Scene& scene, Viewport& viewport, const std::string& filepath) {
  std::ofstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file for writing: " << filepath << std::endl;
    return;
  }

  cereal::PortableBinaryOutputArchive oarchive(file);
  oarchive(scene, viewport);

  file.close();
  std::cout << "[Project] Saved to " << filepath << std::endl;

  loadedFile = true;
  loadedFilePath = filepath;
}

void ProjectData::saveProjectFile(Scene& scene, Viewport& viewport) {
  if (!loadedFile)
    return;
  saveProjectFile(scene, viewport, loadedFilePath);
}

void ProjectData::loadProjectFile(Scene& scene, Viewport& viewport, const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file for reading: " << filepath << std::endl;
    return;
  }

  cereal::PortableBinaryInputArchive oarchive(file);
  oarchive(scene, viewport);

  file.close();
  std::cout << "[Project] Loaded " << filepath << std::endl;

  loadedFile = true;
  loadedFilePath = filepath;
}

bool ProjectData::hasLoadedProjectFile() const { return loadedFile; }
