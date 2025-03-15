#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

namespace ed = ax::NodeEditor;

class SdfNodeEditor {
public:
  SdfNodeEditor();
  ~SdfNodeEditor();

  void show();

  template <typename NodeType> void addNode() { nodes.push_back(new NodeType(getNextId())); }

  std::string generateGlslCode() const;

private:
  std::vector<Node*> nodes;
  std::vector<Link> links;

  ed::EditorContext* editor = nullptr;

  Node* output;

  int nextId = 1;

  int getNextId();

  Node* findNode(ed::NodeId id);
  Pin* findPin(ed::PinId id);
  bool isInvalidPinLink(Pin* a, Pin* b);
  bool linkExists(ed::PinId startID, ed::PinId endID);

  void manageCreation();
  void manageDeletion();
};

// FIXME: Move elsewhere
static SdfNodeEditor sdfNodeEditor;

#endif
