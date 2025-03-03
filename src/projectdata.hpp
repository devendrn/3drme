#ifndef PROJECTDATA_H
#define PROJECTDATA_H

#include "scene.hpp"
#include "viewport.hpp"

class ProjectData {
public:
  ProjectData();

  void saveProjectFile(Scene& scene, Viewport& viewport);
  void saveProjectFile(Scene& scene, Viewport& viewport, const std::string& filepath);
  void loadProjectFile(Scene& scene, Viewport& viewport, const std::string& filepath);

  bool hasLoadedProjectFile() const;

  // void loadPrefFile();
  // void savePrefFile();
private:
  bool loadedFile = false;
  std::string loadedFilePath;
};

#endif
