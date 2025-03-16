#include <algorithm>
#include <string>

#include "imgui_node_editor.h"
#include "node_graph.hpp"
#include "nodes.hpp"

SdfNodeEditor::SdfNodeEditor() {
  ed::Config config;
  editor = ed::CreateEditor(&config);
  output = new SurfaceOutputNode(nextId++);
  nodes.push_back(output);
}

SdfNodeEditor::~SdfNodeEditor() {
  ed::DestroyEditor(editor);
  for (Node* node : nodes)
    delete node;
}

void SdfNodeEditor::show() {
  ed::SetCurrentEditor(editor);
  ed::Begin("SDF Node Editor");

  for (auto* node : nodes)
    node->draw();

  for (auto& link : links)
    ed::Link(link.ID, link.StartPinID, link.EndPinID);

  manageCreation();
  manageDeletion();

  ed::End();
}

std::string SdfNodeEditor::generateGlslCode() const { return output->generateGlsl(); }

unsigned long SdfNodeEditor::getNextId() { return nextId++; }

Node* SdfNodeEditor::findNode(ed::NodeId id) {
  for (auto& node : nodes) {
    if (node->ID == id)
      return node;
  }
  return nullptr;
}

Pin* SdfNodeEditor::findPin(ed::PinId id) {
  if (!id)
    return nullptr;

  for (auto& node : nodes) {
    for (auto& pin : node->inputs) {
      if (pin.ID == id)
        return &pin;
    }
    for (auto& pin : node->outputs) {
      if (pin.ID == id)
        return &pin;
    }
  }

  return nullptr;
}

bool SdfNodeEditor::isInvalidPinLink(Pin* a, Pin* b) {
  return a == b || a->Kind == b->Kind || a->Type != b->Type || a->node == b->node || //
         (a->Kind != PinKind::Output && b->Kind != PinKind::Output);
}

void SdfNodeEditor::manageCreation() {
  if (ed::BeginCreate()) {
    ed::PinId startPinId = 0;
    ed::PinId endPinId = 0;

    if (ed::QueryNewLink(&startPinId, &endPinId)) {
      Pin* startPin = findPin(startPinId);
      Pin* endPin = findPin(endPinId);

      if (startPin != nullptr && endPin != nullptr) {
        if (isInvalidPinLink(startPin, endPin)) {
          ed::RejectNewItem(ImColor(1.0f, 0.0f, 0.0f), 2.0f);
        } else if (ed::AcceptNewItem()) {
          // [In  Out]--->[In  Out]
          if (startPin->Kind != PinKind::Output) {
            std::swap(startPin, endPin);
            std::swap(startPinId, endPinId);
          }

          int index = -1;
          bool alreadyExists = false;
          for (int i = 0; i < links.size(); i++) {
            if (links[i].EndPinID != endPinId)
              continue;

            index = i;

            if (links[i].StartPinID == startPinId) {
              alreadyExists = true;
              break;
            }

            if (endPin->Kind == PinKind::Input)
              break;
          }

          // TODO: Check for feedback loops
          if (!alreadyExists) {
            if (index > -1 && endPin->Kind == PinKind::Input) {
              if (!endPin->pins.empty())
                endPin->pins[0]->removeLink(endPin);
              links.erase(links.begin() + index);
            }

            startPin->pins.push_back(endPin);
            endPin->pins.push_back(startPin);

            links.emplace_back(getNextId(), startPin->ID, endPin->ID);
          }
        }
      }
    }
  }
  ed::EndCreate();
}

void SdfNodeEditor::manageDeletion() {
  if (ed::BeginDelete()) {
    // delete nodes
    ed::NodeId nodeId = 0;
    while (ed::QueryDeletedNode(&nodeId)) {
      if (!ed::AcceptDeletedItem())
        continue;

      // delete connected links
      for (int i = 0; i < links.size(); i++) {
        if (links[i].StartPinID.Get() == nodeId.Get() || links[i].EndPinID.Get() == nodeId.Get())
          links.erase(links.begin() + i);
      }

      // delete node
      // ignore root output node (i=0)
      for (int i = 1; i < nodes.size(); i++) {
        if (nodes[i]->ID == nodeId) {
          delete nodes[i];
          nodes.erase(nodes.begin() + i);
          break;
        }
      }
    }

    // delete links
    ed::LinkId linkId = 0;
    while (ed::QueryDeletedLink(&linkId)) {
      if (!ed::AcceptDeletedItem())
        continue;

      for (int i = 0; i < links.size(); i++) {
        if (links[i].ID == linkId) {
          Pin* start = findPin(links[i].StartPinID);
          Pin* end = findPin(links[i].EndPinID);
          if (start != nullptr)
            start->removeLink(end);
          if (end != nullptr)
            end->removeLink(start);
          links.erase(links.begin() + i);
          break;
        }
      }
    }
  }
  ed::EndDelete();
}

Node* SdfNodeEditor::createNode(unsigned long id, NodeType type) {
  switch (type) {
  case NodeType::InputPosition:
    return new InputPosNode(id);
  case NodeType::InputTime:
    return new InputTimeNode(id);
  case NodeType::Vec3Scale:
    return new Vec3ScaleNode(id);
  case NodeType::Vec3Translate:
    return new Vec3TranslateNode(id);
  case NodeType::SurfaceBoolean:
    return new SurfaceBooleanNode(id);
  case NodeType::SurfaceCreateBox:
    return new SurfaceCreateBoxNode(id);
  case NodeType::SurfaceOutput:
    return new SurfaceOutputNode(id);
  default:
    return nullptr;
  }
}

void SdfNodeEditor::saveGraph(SerializableGraph& graph) {
  for (auto* node : nodes) {
    auto p = ed::GetNodePosition(node->ID);
    SerializableNode sNode{node->ID.Get(), node->type, p.x, p.y};
    graph.nodes.push_back(sNode);
  }
  for (auto& link : links) {
    SerializableLink sLink{link.ID.Get(), link.StartPinID.Get(), link.EndPinID.Get()};
    graph.links.push_back(sLink);
  }
};

void SdfNodeEditor::loadGraph(SerializableGraph& graph) {
  for (Node* node : nodes)
    delete node;

  nodes.clear();
  links.clear();
  nextId = 2;

  for (auto& serializableNode : graph.nodes) {
    Node* newNode = createNode(serializableNode.ID, serializableNode.type);
    if (newNode == nullptr) {
      std::cerr << "Error: Unknown node type: " << static_cast<int>(serializableNode.type) << "\n";
      delete newNode;
      continue;
    }

    nodes.push_back(newNode);
    ed::SetNodePosition(newNode->ID, ImVec2(serializableNode.px, serializableNode.py));
    nextId = serializableNode.ID + 1;
  }

  for (auto& serializableLink : graph.links) {
    Pin* startPin = findPin(serializableLink.startPinID);
    Pin* endPin = findPin(serializableLink.endPinID);

    if (startPin == nullptr || endPin == nullptr) {
      std::cerr << "Error: Invalid link ids found\n";
      continue;
    }

    startPin->pins.push_back(endPin);
    endPin->pins.push_back(startPin);

    links.emplace_back(serializableLink.ID, serializableLink.startPinID, serializableLink.endPinID);
    nextId = serializableLink.ID + 1;
  }
};
