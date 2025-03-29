#include <fstream>
#include <iostream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <glm/fwd.hpp>

#include "camera.hpp"
#include "node_graph.hpp"
#include "projectdata.hpp"
#include "scene.hpp"
#include "utils/cereal_glm.hpp"
#include "viewport.hpp"

namespace cereal {

template <class Archive> void serialize(Archive& archive, Object& d) { archive(d.id, d.name, d.type, d.matId, d.mode, d.position, d.rotation, d.scale, d.extra); };
template <class Archive> void serialize(Archive& archive, Material& d) { archive(d.color); }
template <class Archive> void serialize(Archive& archive, Scene& d) { archive(d.sceneTree, d.materials); }
template <class Archive> void serialize(Archive& archive, Camera& d) { archive(d.dist, d.yaw, d.pitch, d.fov, d.scale, d.target, d.isOrtho); }
template <class Archive> void serialize(Archive& archive, Viewport& d) { archive(d.camera); }

} // namespace cereal

ProjectData::ProjectData() { nodeEditorContext = ax::NodeEditor::CreateEditor(); };

ProjectData::~ProjectData() { ax::NodeEditor::DestroyEditor(nodeEditorContext); }

void ProjectData::saveProjectFile(Scene& scene, Viewport& viewport, NodeEditor& sdfnodeeditor, const std::string& filepath) {
  std::ofstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file for writing: " << filepath << std::endl;
    return;
  }

  SerializableGraph graph;
  sdfnodeeditor.saveGraph(graph);

  cereal::PortableBinaryOutputArchive oarchive(file);
  oarchive(scene, viewport, graph);

  file.close();
  std::cout << "[Project] Saved to " << filepath << std::endl;

  loadedFile = true;
  loadedFilePath = filepath;
}

void ProjectData::saveProjectFile(Scene& scene, Viewport& viewport, NodeEditor& sdfnodeeditor) {
  if (!loadedFile)
    return;
  saveProjectFile(scene, viewport, sdfnodeeditor, loadedFilePath);
}

void ProjectData::loadProjectFile(Scene& scene, Viewport& viewport, NodeEditor& sdfnodeeditor, const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file for reading: " << filepath << std::endl;
    return;
  }

  SerializableGraph graph;

  cereal::PortableBinaryInputArchive oarchive(file);
  oarchive(scene, viewport, graph);

  sdfnodeeditor.loadGraph(graph);

  file.close();
  std::cout << "[Project] Loaded " << filepath << std::endl;

  loadedFile = true;
  loadedFilePath = filepath;
}

bool ProjectData::hasLoadedProjectFile() const { return loadedFile; }
