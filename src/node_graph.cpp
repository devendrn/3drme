#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <imgui_node_editor.h>

#include "imgui.h"
#include "node_graph.hpp"
#include "nodes.hpp"

NodeEditor::NodeEditor() {
  ed::Config config;
  editor = ed::CreateEditor(&config);
  addNode(NodeType::Output);
}

NodeEditor::~NodeEditor() { ed::DestroyEditor(editor); }

void NodeEditor::show() {
  ed::SetCurrentEditor(editor);
  ed::Begin("SDF Node Editor");

  for (auto& node : nodes)
    node->draw();

  for (auto& link : links)
    ed::Link(link.id, link.StartPinId, link.EndPinId);

  manageCreation();
  manageDeletion();

  ed::Suspend();
  if (ed::ShowBackgroundContextMenu()) {
    ImGui::SetCursorScreenPos(ImGui::GetMousePos());
    ImGui::OpenPopup("Create new node");
  }
  if (ImGui::BeginPopup("Create new node")) {
    ImGui::Text("Add node");
    for (const auto& [category, list] : nodeListTree) {
      if (ImGui::BeginMenu(category.c_str())) {
        for (const auto& [name, node] : list) {
          if (ImGui::MenuItem(name.c_str()))
            addNode(node);
        }
        ImGui::EndMenu();
      }
    }
    ImGui::EndPopup();
  }
  ed::Resume();

  ed::End();
}

void NodeEditor::generateGlslCode(std::string& surface, std::string& sky, std::string& lights) const {
  dataPointers.clear();
  if (dataPointers.capacity() < 100)
    dataPointers.reserve(100);
  surface = nodes[0]->generateGlsl(0);
  sky = nodes[0]->generateGlsl(1);
  lights = nodes[0]->generateGlsl(2);
}

Node* NodeEditor::findNode(ed::NodeId id) const {
  for (const auto& node : nodes) {
    if (node->getId() == id)
      return node.get();
  }
  return nullptr;
}

Pin* NodeEditor::findPin(const ed::PinId id) const {
  if (!id)
    return nullptr;

  for (const auto& node : nodes) {
    Pin* p = node->getPin(id);
    if (p != nullptr)
      return p;
  }

  return nullptr;
}

bool NodeEditor::isInvalidPinLink(const Pin* a, const Pin* b) const {
  return a == b || a->kind == b->kind || a->type != b->type || a->node == b->node || //
         (a->kind != PinKind::Output && b->kind != PinKind::Output);
}

void NodeEditor::manageCreation() {
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
          if (startPin->kind != PinKind::Output) {
            std::swap(startPin, endPin);
            std::swap(startPinId, endPinId);
          }

          int index = -1;
          bool alreadyExists = false;
          for (int i = 0; i < links.size(); i++) {
            if (links[i].EndPinId != endPinId)
              continue;

            index = i;

            if (links[i].StartPinId == startPinId) {
              alreadyExists = true;
              break;
            }

            if (endPin->kind == PinKind::Input)
              break;
          }

          if (!alreadyExists && !startPin->node->isAncestor(endPin->node)) {
            if (index > -1 && endPin->kind == PinKind::Input) {
              endPin->clearLinks();
              links.erase(links.begin() + index);
            }

            startPin->addLink(endPin);
            links.emplace_back(nextId++, startPin->id, endPin->id);
          }
        }
      }
    }
  }
  ed::EndCreate();
}

void NodeEditor::manageDeletion() {
  if (ed::BeginDelete()) {
    // delete nodes
    ed::NodeId nodeId = 0;
    while (ed::QueryDeletedNode(&nodeId)) {
      if (!ed::AcceptDeletedItem())
        continue;

      for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i]->getId() == nodeId) {
          if (i == 0) { // reject root output node (i=0)
            ed::RejectDeletedItem();
            continue;
          }

          // delete connected links
          for (int j = 0; j < links.size(); j++) {
            Link& l = links[j];
            const auto& start = findPin(l.StartPinId)->node->getId();
            const auto& end = findPin(l.EndPinId)->node->getId();
            std::cout << "[Node editor] link " << l.id.Get() << ": " << start.Get() << "->" << end.Get() << "\n";
            if (start == nodeId || end == nodeId) {
              std::cout << "[Node editor] Delete link " << l.id.Get() << ": " << start.Get() << "->" << end.Get() << "\n";
              links.erase(links.begin() + j);
              j--; // !! don't increment j
            }
          }

          // delete node
          std::cout << "[Node editor] Delete node " << nodes[i]->getIdLong() << ": " << nodes[i]->getName() << "\n";
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
        Link& l = links[i];
        if (l.id == linkId) {
          Pin* start = findPin(l.StartPinId);
          Pin* end = findPin(l.EndPinId);
          if (start != nullptr)
            start->removeLink(end);
          if (end != nullptr)
            end->removeLink(start);
          std::cout << "[Node editor] Delete link " << l.id.Get() << ": " << start->node->getIdLong() << "->" << end->node->getIdLong() << "\n";
          links.erase(links.begin() + i);
          break;
        }
      }
    }
    ed::EndDelete();
  }
}

std::unique_ptr<Node> NodeEditor::createNode(unsigned long id, NodeType type) {
  if (!nodeDefinitions.contains(type))
    return nullptr;
  return std::make_unique<Node>(id, nodeDefinitions.at(type));
}

void NodeEditor::addNode(NodeType type) {
  if (nodeDefinitions.contains(type)) {
    nodes.push_back(std::make_unique<Node>(nextId, nodeDefinitions.at(type)));
    nextId = nodes.back()->getLastId() + 1;
  }
}

void NodeEditor::saveGraph(SerializableGraph& graph) {
  for (auto& node : nodes) {
    auto [x, y] = ed::GetNodePosition(node->getId());
    SerializableNode sNode{node->getIdLong(), node->getType(), x, y, node->getData()};
    graph.nodes.push_back(std::move(sNode));
  }
  for (auto& link : links) {
    SerializableLink sLink{link.id.Get(), link.StartPinId.Get(), link.EndPinId.Get()};
    graph.links.push_back(std::move(sLink));
  }
};

void NodeEditor::loadGraph(SerializableGraph& graph) {
  nodes.clear();
  links.clear();
  nextId = 1;
  std::cout << "[Node editor] Reset graph\n";

  for (auto& serializableNode : graph.nodes) {
    std::unique_ptr<Node> newNode = createNode(serializableNode.ID, serializableNode.type);
    if (newNode == nullptr) {
      std::cerr << "Error: Unknown node type: " << static_cast<int>(serializableNode.type) << "\n";
      continue;
    }
    newNode->setData(serializableNode.data);
    ed::SetNodePosition(newNode->getId(), ImVec2(serializableNode.px, serializableNode.py));

    nodes.push_back(std::move(newNode));
  }
  for (auto& serializableLink : graph.links) {
    Pin* startPin = findPin(serializableLink.startPinID);
    Pin* endPin = findPin(serializableLink.endPinID);

    if (startPin == nullptr || endPin == nullptr) {
      std::cerr << "Error: Invalid link ids found\n";
      continue;
    }

    startPin->addLink(endPin);
    links.emplace_back(serializableLink.ID, serializableLink.startPinID, serializableLink.endPinID);
  }

  nextId = nodes.back()->getLastId(); // nodes will never be empty
  if (!links.empty())
    nextId = std::max(nextId, links.back().id.Get());

  nextId++;
};

const std::vector<std::unique_ptr<Node>>& NodeEditor::getNodes() const { return nodes; }

void NodeEditor::goToNode(ed::NodeId id) {
  ed::SelectNode(id);
  ed::NavigateToSelection();
}
