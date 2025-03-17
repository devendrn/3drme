#ifndef NODE_GRAPH_H
#define NODE_GRAPH_H

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

namespace ed = ax::NodeEditor;

using NodeData = std::variant<               //
    std::monostate,                          //
    Vec3ScaleNode, Vec3TranslateNode,        //
    SurfaceBooleanNode, SurfaceCreateBoxNode //
    >;

struct SerializableNode {
  unsigned long ID;
  NodeType type;
  float px, py;
  NodeData data;
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

// FIXME: Id generation for pins, nodes, links

class SdfNodeEditor {
public:
  SdfNodeEditor();
  ~SdfNodeEditor();

  void show();

  template <typename NodeType> void addNode() { nodes.push_back(std::make_unique<NodeType>(getNextId())); }

  std::string generateGlslCode() const;

  void saveGraph(SerializableGraph& graph);
  void loadGraph(SerializableGraph& graph);

private:
  std::vector<std::unique_ptr<Node>> nodes;
  std::vector<Link> links;

  ed::EditorContext* editor = nullptr;

  Node* output;

  unsigned long nextId = 1;

  unsigned long getNextId();

  Node* findNode(ed::NodeId id) const;
  Pin* findPin(ed::PinId id) const;
  bool isInvalidPinLink(Pin* a, Pin* b) const;

  void manageCreation();
  void manageDeletion();

  std::unique_ptr<Node> createNode(unsigned long id, NodeType type);

  NodeData getNodeData(const Node* node) const;
  void setNodeData(Node* node, const NodeData& data);
};

// FIXME: Move elsewhere
static SdfNodeEditor sdfNodeEditor;

#endif
