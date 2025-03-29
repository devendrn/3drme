#ifndef PROJECTDATA_H
#define PROJECTDATA_H

#include <imgui_node_editor.h>

#include "node_graph.hpp"
#include "scene.hpp"
#include "viewport.hpp"

class ProjectData {
public:
  ProjectData();
  ~ProjectData();

  void saveProjectFile(Scene& scene, Viewport& viewport, NodeEditor& sdfnodeeditor);
  void saveProjectFile(Scene& scene, Viewport& viewport, NodeEditor& sdfnodeeditor, const std::string& filepath);
  void loadProjectFile(Scene& scene, Viewport& viewport, NodeEditor& sdfnodeeditor, const std::string& filepath);

  bool hasLoadedProjectFile() const;

  ax::NodeEditor::EditorContext* nodeEditorContext;

  // void loadPrefFile();
  // void savePrefFile();
private:
  bool loadedFile = false;
  std::string loadedFilePath;
};

#endif
