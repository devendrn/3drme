#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <memory>
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
  std::vector<float> data;
  template <class Archive> void serialize(Archive& archive) { archive(ID, type, px, py, data); }
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

class SdfNodeEditor {
public:
  SdfNodeEditor();
  ~SdfNodeEditor();

  void show();

  void generateGlslCode(std::string& surface, std::string& sky, std::string& lights) const;

  void saveGraph(SerializableGraph& graph);
  void loadGraph(SerializableGraph& graph);

  void addNode(NodeType type);

private:
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<Link> links;

  ed::EditorContext* editor = nullptr;

  unsigned long nextId = 1;

  Node* findNode(ed::NodeId id) const;
  Pin* findPin(ed::PinId id) const;
  bool isInvalidPinLink(const Pin* a, const Pin* b) const;

  void manageCreation();
  void manageDeletion();

  std::unique_ptr<Node> createNode(unsigned long id, NodeType type);
};

#endif
