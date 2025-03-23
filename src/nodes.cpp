#include <algorithm>
#include <format>
#include <iostream>
#include <string>

#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

Pin::Pin(unsigned long id, const char* name, PinType type, PinKind kind, Node* node = nullptr) : id(id), node(node), name(name), type(type), kind(kind) {}

void Pin::removeLink(const Pin* target) {
  auto index = std::find(pins.begin(), pins.end(), target);
  pins.erase(index);
}

void Pin::addLink(Pin* target) {
  pins.push_back(target);
  target->pins.push_back(this);
}

void Pin::clearLinks() {
  for (auto& linkedPin : pins)
    linkedPin->removeLink(this);
  pins.clear();
}

Node::Node(unsigned long id, const NodeDefinition& definition) : id(id), definition(definition), size(0, 0), lastId(id) { //
  std::cout << "[Node editor] Create node " << id << ": " << definition.name << "\n";
  for (const auto& pinDef : definition.inputs)
    inputs.emplace_back(++lastId, pinDef.name.c_str(), pinDef.type, pinDef.kind, this);
  for (const auto& pinDef : definition.outputs)
    outputs.emplace_back(++lastId, pinDef.name.c_str(), pinDef.type, pinDef.kind, this);
  data = definition.data;
}

Node::~Node() {
  for (Pin& pin : inputs)
    pin.clearLinks();
  for (Pin& pin : outputs)
    pin.clearLinks();
}

bool Node::isAncestor(Node* target) const {
  if (this == target)
    return true;

  for (const auto& pin : this->inputs) {
    for (const auto& p : pin.pins) {
      if (p->node->isAncestor(target))
        return true;
    }
  }

  return false;
}

Pin* Node::getPin(ed::PinId id) {
  for (auto& pin : inputs) {
    if (pin.id == id)
      return &pin;
  }
  for (auto& pin : outputs) {
    if (pin.id == id)
      return &pin;
  }
  return nullptr;
}

void Node::addInputPin(const char* name, PinType type, bool multi) { //
  inputs.emplace_back(++lastId, name, type, multi ? PinKind::InputMulti : PinKind::Input, this);
}

void Node::addOutputPin(const char* name, PinType type) { //
  outputs.emplace_back(++lastId, name, type, PinKind::Output, this);
}

std::string Node::pin0GenerateGlsl(int pinIndex, std::string defaultCode) const {
  if (inputs[pinIndex].pins.empty())
    return defaultCode;
  return inputs[pinIndex].pins[0]->node->generateGlsl();
}

void Node::draw() {
  ed::BeginNode(id);
  {
    // TODO: Header color, centering
    ImGui::PushID(&id);
    ImGui::PushItemWidth(102);

    ImGui::Text("%s", definition.name.c_str());
    drawContent();

    ImGui::PopItemWidth();
    ImGui::PopID();
  }
  ed::EndNode();
}

ImColor getPinColor(PinType type) {
  switch (type) {
  case PinType::Surface:
    return ImColor(1.0f, 1.0f, 0.3f);
  case PinType::Vec3:
    return ImColor(1.0f, 0.3f, 1.0f);
  case PinType::Float:
    return ImColor(0.8f, 0.8f, 0.8f);
  default:
    return ImColor(1.0f, 0.3f, 0.3f);
  }
}

void drawBaseOutput(Pin& pin) {
  float textWidth = ImGui::CalcTextSize(pin.name.c_str()).x;
  ImGui::Dummy(ImVec2(70.0f - textWidth, 8.0f));
  ImGui::SameLine();
  ed::BeginPin(pin.id, ed::PinKind::Output);
  {
    ImGui::Text("%s", pin.name.c_str());
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(16, 16));
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    // ed::PinPivotSize(ImVec2(0, 0));
    ImColor pinColor = getPinColor(pin.type);
    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin() + ImVec2(3, 3), ImGui::GetItemRectMin() + ImVec2(13, 13), pinColor, 6);
  }
  ed::EndPin();
}

void drawBaseInput(Pin& pin, std::function<void()> inner = [] {}) {
  ed::BeginPin(pin.id, ed::PinKind::Input);
  {
    float extraHeight = pin.kind == PinKind::InputMulti ? 2 : 0;
    ImGui::Dummy(ImVec2(16, 16));
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ImColor pinColor = getPinColor(pin.type);
    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin() + ImVec2(3, 3 - extraHeight), ImGui::GetItemRectMin() + ImVec2(13 - extraHeight, 13 + extraHeight), pinColor, 6);
    ImGui::SameLine();
    ImGui::Text("%s", pin.name.c_str());
  }
  ed::EndPin();
  if (pin.pins.empty())
    inner();
}

void Node::drawContent() { definition.drawContent(this); }
std::string Node::generateGlsl(int variant) const { return definition.generateGlsl(this, variant); }
std::vector<float> Node::getData() const { return data; }
void Node::setData(const std::vector<float>& data) { this->data = data; }

std::string toVec3String(float x, float y, float z) { return std::format("vec3({},{},{})", x, y, z); }
std::string toVec3String(const float* data) { return toVec3String(*(data), *(data + 1), *(data + 2)); }

// use constexpr ?
std::map<NodeType, NodeDefinition> initDefinitions() {
  std::map<NodeType, NodeDefinition> defs;
  {
    NodeDefinition nd{NodeType::Output, "Output"};
    nd.addInput("Surface", PinType::Surface);
    nd.addInput("Sky", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      drawBaseInput(node->inputs[0]);
      drawBaseInput(node->inputs[1]);
    });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string {
      // surface variant
      if (variant == 0) {
        const Pin& i0 = node->inputs[0];
        if (i0.pins.empty())
          return "";
        return std::format("s={};", i0.pins[0]->node->generateGlsl());
      }
      // sky variant
      const Pin& i1 = node->inputs[1];
      if (i1.pins.empty())
        return "";
      return std::format("s={};", i1.pins[0]->node->generateGlsl());
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateSphere, "Surface Sphere"};
    const int colLoc = 0;
    const int posLoc = 3;
    const int radiusLoc = 6;
    nd.initializeData({
        1, 1, 1, // col
        0, 0, 0, // pos
        1        // radius
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Size", PinType::Vec3);
    nd.addInput("Roundness", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      drawBaseOutput(node->outputs[0]);
      drawBaseInput(node->inputs[0], [&] {
        ImGui::ColorEdit3("##col", &node->data[colLoc]);
        ImGui::Dummy(ImVec2(0, 4));
      });
      drawBaseInput(node->inputs[1], [&] {
        ImGui::DragFloat3("##pos", &node->data[posLoc]);
        ImGui::Dummy(ImVec2(0, 4));
      });
      drawBaseInput(node->inputs[2], [&] { ImGui::DragFloat("##rou", &node->data[radiusLoc]); });
    });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string {
      std::string colCode = node->pin0GenerateGlsl(0, toVec3String(&node->data[colLoc]));
      std::string posCode = node->pin0GenerateGlsl(1, "pos+" + toVec3String(&node->data[posLoc]));
      std::string radiusCode = node->pin0GenerateGlsl(2, std::format("{:.3f}", node->data[radiusLoc]));
      return std::format("Surface(sdfSphere({},{}),{},0.0)", posCode, radiusCode, colCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateBox, "Surface Box"};
    const int colLoc = 0;
    const int posLoc = 3;
    const int sizeLoc = 6;
    const int roundnessLoc = 9;
    nd.initializeData({
        1, 1, 1, // col
        0, 0, 0, // pos
        1, 1, 1, // size
        0        // roundness
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Size", PinType::Vec3);
    nd.addInput("Roundness", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      drawBaseOutput(node->outputs[0]);
      drawBaseInput(node->inputs[0], [&] {
        ImGui::ColorEdit3("##col", &node->data[colLoc]);
        ImGui::Dummy(ImVec2(0, 4));
      });
      drawBaseInput(node->inputs[1], [&] {
        ImGui::DragFloat3("##pos", &node->data[posLoc]);
        ImGui::Dummy(ImVec2(0, 4));
      });
      drawBaseInput(node->inputs[2], [&] {
        ImGui::DragFloat3("##bou", &node->data[sizeLoc]);
        ImGui::Dummy(ImVec2(0, 4));
      });
      drawBaseInput(node->inputs[3], [&] { ImGui::DragFloat("##rou", &node->data[roundnessLoc]); });
    });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string {
      std::string colCode = node->pin0GenerateGlsl(0, toVec3String(&node->data[colLoc]));
      std::string posCode = node->pin0GenerateGlsl(1, "pos+" + toVec3String(&node->data[posLoc]));
      std::string boundCode = node->pin0GenerateGlsl(2, toVec3String(&node->data[sizeLoc]));
      std::string roundnessCode = node->pin0GenerateGlsl(3, std::format("{:.3f}", node->data[roundnessLoc]));
      return std::format("Surface(sdfBox({},{},{}),{},0.0)", posCode, boundCode, roundnessCode, colCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    const int typeLoc = 0;
    const int smoothLoc = 1;
    NodeDefinition nd{NodeType::SurfaceBoolean, "Surface Boolean"};
    nd.initializeData({0, 0});
    nd.addInput("Input A", PinType::Surface);
    nd.addInput("Input B,C..", PinType::Surface, true);
    nd.addOutput("Output", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      drawBaseOutput(node->outputs[0]);
      ImGui::DragFloat3("##x", node->data.data());
      ImVec2 size = ImVec2(10, 14);
      ImGui::Dummy(ImVec2(20, 5));
      float& typef = node->data[typeLoc];
      ImGui::SameLine();
      if (ImGui::Selectable("U", typef == 0.0f, 0, size))
        typef = 0.0f;
      ImGui::SameLine();
      if (ImGui::Selectable("D", typef == 1.0f, 0, size))
        typef = 1.0f;
      ImGui::SameLine();
      if (ImGui::Selectable("I", typef == 2.0f, 0, size))
        typef = 2.0f;
      ImGui::DragFloat("##smooth", &node->data[smoothLoc], 0.01f, 0.0, 1.0f);
      drawBaseInput(node->inputs[0]);
      drawBaseInput(node->inputs[1]);
    });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string {
      const Pin& i0 = node->inputs[0];
      const Pin& i1 = node->inputs[1];
      if (i0.pins.empty())
        return "Surface(FLOAT_MAX,vec3(0),0.0)";
      std::string result = i0.pins[0]->node->generateGlsl();
      if (i1.pins.empty())
        return result;
      float typef = node->data[typeLoc];
      float smooth = node->data[smoothLoc];
      std::string func;
      std::string end = ")";
      if (typef == 0.0f) {
        func = "uSurf";
        if (smooth > 0.0)
          end = "," + std::to_string(smooth) + ")";
      } else if (typef == 1.0f)
        func = "dSurf";
      else if (typef == 2.0f)
        func = "iSurf";
      auto l = i1.pins.size();
      for (int i = 0; i < l; i++)
        result = std::format("{}({},{}{}", func, result, i1.pins[i]->node->generateGlsl(), end);
      return result;
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Translate, "Vec3 Translate"};
    nd.initializeData({0, 0, 0});
    nd.addInput("Input", PinType::Vec3);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      drawBaseOutput(node->outputs[0]);
      ImGui::DragFloat3("##x", node->data.data());
      drawBaseInput(node->inputs[0]);
    });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string {
      std::string posCode = node->pin0GenerateGlsl(0, "0.0");
      return std::format("({}+{})", posCode, toVec3String(node->data.data()));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Scale, "Vec3 Scale"};
    nd.initializeData({1, 1, 1});
    nd.addInput("Input", PinType::Vec3);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      drawBaseOutput(node->outputs[0]);
      ImGui::DragFloat3("##x", node->data.data());
      drawBaseInput(node->inputs[0]);
    });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string {
      std::string posCode = node->pin0GenerateGlsl(0, "0.0");
      return std::format("({}*{})", posCode, toVec3String(node->data.data()));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::InputTime, "Time"};
    nd.addOutput("", PinType::Float);
    nd.setDrawContent([](Node* node) { drawBaseOutput(node->outputs[0]); });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string { return "t"; });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::InputPosition, "Position"};
    nd.addOutput("", PinType::Vec3);
    nd.setDrawContent([](Node* node) { drawBaseOutput(node->outputs[0]); });
    nd.setGenerateGlsl([](const Node* node, int variant) -> std::string { return "pos"; });
    defs.insert({nd.type, nd});
  }

  return std::move(defs);
}

const std::map<NodeType, NodeDefinition> nodeDefinitions = initDefinitions();
