#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <string>
#include <vector>

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

namespace ed = ax::NodeEditor;

struct SerializableNode {
  unsigned long ID;
  NodeType type;
  float px, py;
  template <class Archive> void serialize(Archive& archive) { archive(ID, type, px, py); }
};

struct SerializableLink {
  unsigned long ID, startPinID, endPinID;
  template <class Archive> void serialize(Archive& archive) { archive(ID, startPinID, endPinID); }
};

struct SerializableGraph {
  std::vector<SerializableNode> nodes;
  std::vector<SerializableLink> links;
  template <class Archive> void serialize(Archive& archive) { archive(nodes, links); }
};

// FIXME: Id generation for pins, nodes, links

class SdfNodeEditor {
public:
  SdfNodeEditor();
  ~SdfNodeEditor();

  void show();

  template <typename NodeType> void addNode() { nodes.push_back(new NodeType(getNextId())); }

  std::string generateGlslCode() const;

  void saveGraph(SerializableGraph& graph);
  void loadGraph(SerializableGraph& graph);

private:
  std::vector<Node*> nodes;
  std::vector<Link> links;

  ed::EditorContext* editor = nullptr;

  Node* output;

  unsigned long nextId = 1;

  unsigned long getNextId();

  Node* findNode(ed::NodeId id);
  Pin* findPin(ed::PinId id);
  bool isInvalidPinLink(Pin* a, Pin* b);

  void manageCreation();
  void manageDeletion();

  Node* createNode(unsigned long id, NodeType type);
};

// FIXME: Move elsewhere
static SdfNodeEditor sdfNodeEditor;

#endif
