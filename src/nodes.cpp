#include <algorithm>
#include <format>
#include <iostream>
#include <string>

#include <imgui.h>
#include <imgui_node_editor.h>
#include <misc/cpp/imgui_stdlib.h>

#include "imgui_internal.h"
#include "nodes.hpp"

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

std::string Pin::generateGlsl() const { return node->generateGlsl(id.Get()); }

Node::Node(unsigned long id, const NodeDefinition& definition) : id(id), definition(definition), lastId(id) { //
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
  return inputs[pinIndex].pins[0]->generateGlsl();
}

void Node::draw() {
  ed::BeginNode(id);
  {
    ImGui::PushID(&id);
    ImGui::PushItemWidth(definition.width);

    ImGui::Text("%s", definition.name.c_str());
    ImGui::Dummy(ImVec2(definition.width - 32, 4));
    drawContent();

    ImGui::PopItemWidth();
    ImGui::PopID();
  }
  ed::EndNode();

  auto style = ed::GetStyle();
  ImVec2 headerMin = ImGui::GetItemRectMin();
  ImVec2 headerMax = ImGui::GetItemRectMax();
  headerMax.y = headerMin.y + 30;
  ImVec2 borderOffset = ImVec2(1, 1) * style.NodeBorderWidth;
  headerMin += borderOffset;
  headerMax -= borderOffset;
  auto* drawList = ed::GetNodeBackgroundDrawList(id);
  drawList->AddRectFilled(headerMin, headerMax, definition.color, style.NodeRounding - style.NodeBorderWidth, ImDrawFlags_RoundCornersTop);
  drawList->AddLine(ImVec2(headerMin.x - borderOffset.x, headerMax.y), headerMax, ImColor(style.Colors[ed::StyleColor_NodeBorder]), style.NodeBorderWidth);
}

const ed::NodeId& Node::getId() const { return id; };
unsigned long Node::getIdLong() const { return id.Get(); };
unsigned long Node::getLastId() const { return lastId; };
NodeType Node::getType() const { return definition.type; };
const std::string& Node::getName() const { return definition.name; };
const std::vector<Pin>& Node::getInputs() const { return inputs; };
const std::vector<Pin>& Node::getOutputs() const { return outputs; };

void Node::drawContent() { definition.drawContent(this); }

std::string Node::generateGlsl(unsigned long outputPinId) const { return definition.generateGlsl(this, outputPinId); }

std::vector<float> Node::getData() const { return data; }

void Node::setData(const std::vector<float>& data) { this->data = data; }

unsigned long appendDataPtrs(const Node* node) {
  unsigned long index = dataPointers.size();
  for (const auto& x : node->data)
    dataPointers.push_back(&x);
  return index;
}

std::string uNFloat(unsigned long index) { return std::format("uN[{}]", index); }
std::string uNVec3(unsigned long index) { return std::format("vec3(uN[{}],uN[{}],uN[{}])", index, index + 1, index + 2); }

void Node::drawBaseOutput(int index) {
  auto& pin = outputs[index];
  float textWidth = ImGui::CalcTextSize(pin.name.c_str()).x;
  ImGui::Dummy(ImVec2(definition.width - textWidth - 32, 8));
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

void Node::drawBaseInput(int index, std::function<void()> inner) {
  auto& pin = inputs[index];
  ed::BeginPin(pin.id, ed::PinKind::Input);
  {
    float roundness = pin.kind == PinKind::InputMulti ? 1 : 5;
    ImGui::Dummy(ImVec2(14, 16));
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ImColor pinColor = getPinColor(pin.type);
    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin() + ImVec2(3, 3), ImGui::GetItemRectMin() + ImVec2(13, 13), pinColor, roundness);
    ImGui::SameLine();
    ImGui::Text("%s", pin.name.c_str());
  }
  ed::EndPin();
  if (pin.pins.empty())
    inner();
}

std::string toVec3String(float x, float y, float z) { return std::format("vec3({},{},{})", x, y, z); }
std::string toVec3String(const float* data) { return toVec3String(*(data), *(data + 1), *(data + 2)); }

void drawColorEdit(float* data) { ImGui::ColorEdit3("##col", data, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoOptions); }
void drawVec3Edit(const char* id, float* data, float speed = 0.04f, float min = 0.0, float max = 0.0) { ImGui::DragFloat3(id, data, speed, min, max); }
void drawFloatEdit(const char* id, float* data, float speed = 0.01f, float min = 0.0, float max = 1.0) { ImGui::DragFloat(id, data, speed, min, max); }

// use constexpr ?
std::map<NodeType, NodeDefinition> initDefinitions() {
  const float sat = 0.8;
  const float val = 0.3;
  ImColor surfaceColor = ImColor::HSV(0.0, sat, val);
  ImColor vec3Color = ImColor::HSV(0.1, sat, val);
  ImColor floatColor = ImColor::HSV(0.2, sat, val);
  ImColor inputsColor = ImColor::HSV(0.3, sat, val);
  ImColor lightsColor = ImColor::HSV(0.4, sat, val);
  ImColor outputColor = ImColor::HSV(0.5, sat, val);

  std::map<NodeType, NodeDefinition> defs;
  {
    NodeDefinition nd{NodeType::Output, "Output", outputColor, 120};
    nd.addInput("Surface", PinType::Surface);
    nd.addInput("Sky", PinType::Vec3);
    nd.addInput("Lights", PinType::Light, true);
    nd.setDrawContent([](Node* node) {
      node->drawBaseInput(0);
      node->drawBaseInput(1);
      node->drawBaseInput(2);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long variant) -> std::string {
      // surface variant
      if (variant == 0) {
        const Pin& i0 = node->inputs[0];
        if (i0.pins.empty())
          return "";
        return std::format("s={};", i0.pins[0]->generateGlsl());
      }
      // sky variant
      if (variant == 1) {
        const Pin& i1 = node->inputs[1];
        if (i1.pins.empty())
          return "";
        return std::format("s={};", i1.pins[0]->generateGlsl());
      }
      // lights variant
      const Pin& i2 = node->inputs[2];
      if (i2.pins.empty())
        return "";
      std::string code;
      for (int i = 0; i < i2.pins.size(); i++)
        code += "," + i2.pins[i]->generateGlsl();
      return code;
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateSphere, "Surface Sphere", surfaceColor};
    const int colLoc = 0;
    const int roughnessLoc = 3;
    const int posLoc = 4;
    const int radiusLoc = 7;
    nd.initializeData({
        1, 1, 1, // col
        1,       // roughness
        0, 0, 0, // pos
        1,       // radius
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Roughness", PinType::Float);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Radius", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##rough", &node->data[roughnessLoc]); });
      node->drawBaseInput(2, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
      node->drawBaseInput(3, [&] { drawFloatEdit("##radius", &node->data[radiusLoc], 0.03, 0, 1e3); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string roughnessCode = node->pin0GenerateGlsl(1, uNFloat(index + roughnessLoc));
      std::string posCode = node->pin0GenerateGlsl(2, "pos-" + uNVec3(index + posLoc));
      std::string radiusCode = node->pin0GenerateGlsl(3, uNFloat(index + radiusLoc));
      return std::format("Surface(sdfSphere({},{}),{},0.0,{})", posCode, radiusCode, colCode, roughnessCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateBox, "Surface Box", surfaceColor};
    const int colLoc = 0;
    const int roughnessLoc = 3;
    const int posLoc = 4;
    const int sizeLoc = 7;
    const int roundingLoc = 10;
    nd.initializeData({
        1, 1, 1, // col
        1,       // roughness
        0, 0, 0, // pos
        1, 1, 1, // size
        0,       // roundness
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Roughness", PinType::Float);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Size", PinType::Vec3);
    nd.addInput("Rounding", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##rough", &node->data[roughnessLoc]); });
      node->drawBaseInput(2, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
      node->drawBaseInput(3, [&] { drawVec3Edit("##bou", &node->data[sizeLoc]); });
      node->drawBaseInput(4, [&] { drawFloatEdit("##round", &node->data[roundingLoc]); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string roughnessCode = node->pin0GenerateGlsl(1, uNFloat(index + roughnessLoc));
      std::string posCode = node->pin0GenerateGlsl(2, "pos-" + uNVec3(index + posLoc));
      std::string boundCode = node->pin0GenerateGlsl(3, uNVec3(index + sizeLoc));
      std::string roundingCode = node->pin0GenerateGlsl(4, uNFloat(index + roundingLoc));
      return std::format("Surface(sdfBox({},{},{}),{},0.0,{})", posCode, boundCode, roundingCode, colCode, roughnessCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateCylinder, "Surface Cylinder", surfaceColor};
    const int colLoc = 0;
    const int roughnessLoc = 3;
    const int posLoc = 4;
    const int radiusLoc = 7;
    const int heightLoc = 8;
    const int roundingLoc = 9;
    nd.initializeData({
        1, 1, 1, // col
        1,       // roughness
        0, 0, 0, // pos
        1,       // radius
        1,       // height
        0,       // rounding
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Roughness", PinType::Float);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Radius", PinType::Float);
    nd.addInput("Height", PinType::Float);
    nd.addInput("Rounding", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##rough", &node->data[roughnessLoc]); });
      node->drawBaseInput(2, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
      node->drawBaseInput(3, [&] { drawFloatEdit("##radius", &node->data[radiusLoc], 0.03, 0.0, 1e2); });
      node->drawBaseInput(4, [&] { drawFloatEdit("##height", &node->data[heightLoc], 0.03, 0.0, 1e4); });
      node->drawBaseInput(5, [&] { drawFloatEdit("##round", &node->data[roundingLoc], 0.03, 0.0, 1e2); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string roughnessCode = node->pin0GenerateGlsl(1, uNFloat(index + roughnessLoc));
      std::string posCode = node->pin0GenerateGlsl(2, "pos-" + uNVec3(index + posLoc));
      std::string radiusCode = node->pin0GenerateGlsl(3, uNFloat(index + radiusLoc));
      std::string heightCode = node->pin0GenerateGlsl(4, uNFloat(index + heightLoc));
      std::string roundingCode = node->pin0GenerateGlsl(5, uNFloat(index + roundingLoc));
      return std::format("Surface(sdfCylinder({},{},{},{}),{},0.0,{})", posCode, radiusCode, heightCode, roundingCode, colCode, roughnessCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateTorus, "Surface Torus", surfaceColor};
    const int colLoc = 0;
    const int roughnessLoc = 3;
    const int posLoc = 4;
    const int radiusLoc = 7;
    const int thicknessLoc = 8;
    nd.initializeData({
        1, 1, 1, // col
        1,       // roughness
        0, 0, 0, // pos
        0.8,     // ring radius
        0.4,     // thickness
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Roughness", PinType::Float);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Ring radius", PinType::Float);
    nd.addInput("Ring thickness", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##rough", &node->data[roughnessLoc]); });
      node->drawBaseInput(2, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
      node->drawBaseInput(3, [&] { drawFloatEdit("##radius", &node->data[radiusLoc], 0.03, 0.0, 1e2); });
      node->drawBaseInput(4, [&] { drawFloatEdit("##thickness", &node->data[thicknessLoc], 0.03, 0.0, 1e2); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string roughnessCode = node->pin0GenerateGlsl(1, uNFloat(index + roughnessLoc));
      std::string posCode = node->pin0GenerateGlsl(2, "pos-" + uNVec3(index + posLoc));
      std::string radiusCode = node->pin0GenerateGlsl(3, uNFloat(index + radiusLoc));
      std::string thicknessCode = node->pin0GenerateGlsl(4, uNFloat(index + thicknessLoc));
      return std::format("Surface(sdfTorus({},{},{}),{},0.0,{})", posCode, radiusCode, thicknessCode, colCode, roughnessCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreateCone, "Surface Cone", surfaceColor};
    const int colLoc = 0;
    const int roughnessLoc = 3;
    const int posLoc = 4;
    const int heightLoc = 7;
    const int topRadiusLoc = 8;
    const int bottomRadiusLoc = 9;
    const int roundingLoc = 10;
    nd.initializeData({
        1, 1, 1, // col
        1,       // roughness
        0, 0, 0, // pos
        1,       // height
        0,       // top radius
        1,       // bottom radius
        0,       // rounding
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Roughness", PinType::Float);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Height", PinType::Float);
    nd.addInput("Top radius", PinType::Float);
    nd.addInput("Bottom radius", PinType::Float);
    nd.addInput("Rounding", PinType::Float);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##rough", &node->data[roughnessLoc]); });
      node->drawBaseInput(2, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
      node->drawBaseInput(3, [&] { drawFloatEdit("##height", &node->data[heightLoc], 0.03, 0.0, 1e2); });
      node->drawBaseInput(4, [&] { drawFloatEdit("##topradius", &node->data[topRadiusLoc], 0.03, 0.0, 1e2); });
      node->drawBaseInput(5, [&] { drawFloatEdit("##bottomradius", &node->data[bottomRadiusLoc], 0.03, 0.0, 1e2); });
      node->drawBaseInput(6, [&] { drawFloatEdit("##rounding", &node->data[roundingLoc], 0.03, 0.0, 1e2); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string roughnessCode = node->pin0GenerateGlsl(1, uNFloat(index + roughnessLoc));
      std::string posCode = node->pin0GenerateGlsl(2, "pos-" + uNVec3(index + posLoc));
      std::string heightCode = node->pin0GenerateGlsl(3, uNFloat(index + heightLoc));
      std::string topRadiusCode = node->pin0GenerateGlsl(4, uNFloat(index + topRadiusLoc));
      std::string bottomRadiusCode = node->pin0GenerateGlsl(5, uNFloat(index + bottomRadiusLoc));
      std::string roundingCode = node->pin0GenerateGlsl(6, uNFloat(index + roundingLoc));
      return std::format("Surface(sdfCappedCone({},{},{},{},{}),{},0.0,{})", posCode, heightCode, topRadiusCode, bottomRadiusCode, roundingCode, colCode, roughnessCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::SurfaceCreatePlane, "Surface Plane", surfaceColor};
    const int colLoc = 0;
    const int roughnessLoc = 3;
    const int posLoc = 4;
    const int normalLoc = 7;
    nd.initializeData({
        1, 1, 1, // col
        1,       // roughness
        0, 0, 0, // pos
        0, 1, 0, // normal
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Roughness", PinType::Float);
    nd.addInput("Postion", PinType::Vec3);
    nd.addInput("Normal", PinType::Vec3);
    nd.addOutput("", PinType::Surface);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##rough", &node->data[roughnessLoc]); });
      node->drawBaseInput(2, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
      node->drawBaseInput(3, [&] { drawVec3Edit("##nrm", &node->data[normalLoc], 0.01, -1, 1); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string roughnessCode = node->pin0GenerateGlsl(1, uNFloat(index + roughnessLoc));
      std::string posCode = node->pin0GenerateGlsl(2, "pos-" + uNVec3(index + posLoc));
      std::string normalCode = node->pin0GenerateGlsl(3, uNVec3(index + normalLoc));
      return std::format("Surface(sdfPlane({},{}),{},0.0,{})", posCode, normalCode, colCode, roughnessCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    const int typeLoc = 0;
    const int smoothLoc = 1;
    NodeDefinition nd{NodeType::SurfaceBoolean, "Surface Boolean", surfaceColor, 100};
    nd.initializeData({0, 0});
    nd.addInput("Input A", PinType::Surface);
    nd.addInput("Input B,C..", PinType::Surface, true);
    nd.addOutput("Output", PinType::Surface);
    nd.setDrawContent([&](Node* node) {
      node->drawBaseOutput(0);
      ImVec2 size = ImVec2(10, 14);
      ImGui::Dummy(ImVec2(22, 5));
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
      drawFloatEdit("##smooth", &node->data[smoothLoc], 0.01f, 0, 1e2);
      node->drawBaseInput(0);
      node->drawBaseInput(1);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      const Pin& i0 = node->inputs[0];
      const Pin& i1 = node->inputs[1];
      if (i0.pins.empty())
        return "Surface(FLOAT_MAX,vec3(0),0.0,0.0)";
      std::string result = i0.pins[0]->generateGlsl();
      if (i1.pins.empty())
        return result;
      float typef = node->data[typeLoc];
      float smooth = node->data[smoothLoc];
      std::string func;
      std::string end = ")";
      if (typef == 0.0f) {
        func = "uSurf";
      } else if (typef == 1.0f)
        func = "dSurf";
      else if (typef == 2.0f)
        func = "iSurf";
      if (smooth > 0.0)
        end = "," + uNFloat(index + smoothLoc) + ")";
      auto l = i1.pins.size();
      for (int i = 0; i < l; i++)
        result = std::format("{}({},{}{}", func, result, i1.pins[i]->generateGlsl(), end);
      return result;
    });
    defs.insert({nd.type, nd});
  }
  {
    const int mixLoc = 0;
    NodeDefinition nd{NodeType::SurfaceMix, "Surface Mix", surfaceColor, 100};
    nd.initializeData({0});
    nd.addInput("Input A", PinType::Surface);
    nd.addInput("Input B", PinType::Surface);
    nd.addOutput("Output", PinType::Surface);
    nd.setDrawContent([&](Node* node) {
      node->drawBaseOutput(0);
      drawFloatEdit("##smooth", &node->data[mixLoc]);
      node->drawBaseInput(0);
      node->drawBaseInput(1);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string surfACode = node->pin0GenerateGlsl(0, "Surface(FLOAT_MAX,vec3(0),0.0,0.0)");
      std::string surfBCode = node->pin0GenerateGlsl(1, "Surface(FLOAT_MAX,vec3(0),0.0,0.0)");
      return std::format("mSurf({},{},{})", surfACode, surfBCode, uNFloat(index + mixLoc));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Float, "Float", floatColor, 70};
    nd.initializeData({0});
    nd.addOutput("Output", PinType::Float);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      drawFloatEdit("##x", node->data.data());
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      return uNFloat(index);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::FloatCode, "Float Code", floatColor, 300};
    nd.addOutput("Output", PinType::Float);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      ImGui::InputText("##code", &node->code);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string { return node->code; });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::FloatSine, "Float Sine", floatColor, 70};
    nd.initializeData({1});
    nd.addInput("Input", PinType::Float);
    nd.addOutput("Output", PinType::Float);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      drawFloatEdit("##x", node->data.data());
      node->drawBaseInput(0);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string { return std::format("sin({}*{})", node->data[0], node->pin0GenerateGlsl(0, "0.0")); });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3, "Vec3", vec3Color};
    nd.initializeData({0, 0, 0});
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      drawVec3Edit("##x", node->data.data());
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      return uNVec3(index);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Code, "Vec3 Code", vec3Color, 300};
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      ImGui::InputText("##code", &node->code);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string { return node->code; });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Math, "Vec3 Math", vec3Color, 80};
    nd.initializeData({0});
    nd.addInput("A", PinType::Vec3);
    nd.addInput("B", PinType::Vec3);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      ImVec2 size = ImVec2(10, 14);
      float& typef = node->data[0];
      if (ImGui::Selectable("+", typef == 0.0f, 0, size))
        typef = 0.0f;
      ImGui::SameLine();
      if (ImGui::Selectable("*", typef == 1.0f, 0, size))
        typef = 1.0f;
      ImGui::SameLine();
      if (ImGui::Selectable("/", typef == 2.0f, 0, size))
        typef = 2.0f;
      node->drawBaseInput(0);
      node->drawBaseInput(1);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      const float& typef = node->data[0];
      char op = typef == 0.0f ? '+' : '*';
      if (typef == 2.0)
        op = '/';
      return std::format("({}{}{})", node->pin0GenerateGlsl(0, "0.0"), op, node->pin0GenerateGlsl(1, "0.0"));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Translate, "Vec3 Translate", vec3Color};
    nd.initializeData({0, 0, 0});
    nd.addInput("Input", PinType::Vec3);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      drawVec3Edit("##x", node->data.data());
      node->drawBaseInput(0);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string posCode = node->pin0GenerateGlsl(0, "0.0");
      return std::format("({}+{})", posCode, uNVec3(index));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Scale, "Vec3 Scale", vec3Color};
    nd.initializeData({1, 1, 1});
    nd.addInput("Input", PinType::Vec3);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      drawVec3Edit("##x", node->data.data());
      node->drawBaseInput(0);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string posCode = node->pin0GenerateGlsl(0, "0.0");
      return std::format("({}*{})", posCode, uNVec3(index));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Rotate, "Vec3 Rotate", vec3Color};
    nd.initializeData({0, 0, 0});
    nd.addInput("Input", PinType::Vec3);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      drawVec3Edit("##x", node->data.data());
      node->drawBaseInput(0);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string posCode = node->pin0GenerateGlsl(0, "0.0");
      return std::format("({}*rmat({}))", posCode, uNVec3(index));
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Split, "Vec3 Split", vec3Color, 100};
    nd.addInput("Input", PinType::Vec3);
    nd.addOutput("X", PinType::Float);
    nd.addOutput("Y", PinType::Float);
    nd.addOutput("Y", PinType::Float);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseOutput(1);
      node->drawBaseOutput(2);
      node->drawBaseInput(0);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      const auto& p = node->inputs[0].pins;
      if (p.empty())
        return "0.0";
      if (outputPinId == node->outputs[0].id.Get())
        return p[0]->generateGlsl() + ".x";
      if (outputPinId == node->outputs[1].id.Get())
        return p[0]->generateGlsl() + ".y";
      return p[0]->generateGlsl() + ".z";
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::Vec3Combine, "Vec3 Combine", vec3Color, 100};
    nd.addInput("X", PinType::Float);
    nd.addInput("Y", PinType::Float);
    nd.addInput("Y", PinType::Float);
    nd.addOutput("Output", PinType::Vec3);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      node->drawBaseInput(0);
      node->drawBaseInput(1);
      node->drawBaseInput(2);
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      auto x = node->pin0GenerateGlsl(0, "0.0");
      auto y = node->pin0GenerateGlsl(1, "0.0");
      auto z = node->pin0GenerateGlsl(2, "0.0");
      return std::format("vec3({},{},{})", x, y, z);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::LightPoint, "Light Point", lightsColor};
    const int colLoc = 0;
    const int posLoc = 3;
    const int intensityLoc = 6;
    const int radiusLoc = 7;
    const int stepsLoc = 8;
    nd.initializeData({
        1, 1, 1, //
        0, 1, 0, //
        10,      //
        0.1,     //
        16       //
    });
    nd.addInput("Color", PinType::Vec3);
    nd.addInput("Intensity", PinType::Float);
    nd.addInput("Radius", PinType::Float);
    nd.addInput("Position", PinType::Vec3);
    nd.addOutput("", PinType::Light);
    nd.setDrawContent([](Node* node) {
      node->drawBaseOutput(0);
      int stepsInt = static_cast<int>(node->data[stepsLoc]);
      ImGui::Text("Steps");
      ImGui::DragInt("##steps", &stepsInt, 1, 0, 128);
      node->data[stepsLoc] = static_cast<float>(stepsInt);
      node->drawBaseInput(0, [&] { drawColorEdit(&node->data[colLoc]); });
      node->drawBaseInput(1, [&] { drawFloatEdit("##intensity", &node->data[intensityLoc], 0.05, 0, 1e4); });
      node->drawBaseInput(2, [&] { drawFloatEdit("##radius", &node->data[radiusLoc], 0.01, 0.0, 0.5); });
      node->drawBaseInput(3, [&] { drawVec3Edit("##pos", &node->data[posLoc]); });
    });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string {
      unsigned long index = appendDataPtrs(node);
      std::string colorCode = node->pin0GenerateGlsl(0, uNVec3(index + colLoc));
      std::string intensityCode = node->pin0GenerateGlsl(1, uNFloat(index + intensityLoc));
      std::string radiusCode = node->pin0GenerateGlsl(2, uNFloat(index + radiusLoc));
      std::string posCode = node->pin0GenerateGlsl(3, uNVec3(index + posLoc));
      return std::format("Light({},{}*{},{},{})", posCode, intensityCode, colorCode, static_cast<int>(node->data[stepsLoc]), radiusCode);
    });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::InputTime, "Time", inputsColor, 70};
    nd.addOutput("", PinType::Float);
    nd.setDrawContent([](Node* node) { node->drawBaseOutput(0); });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string { return "t"; });
    defs.insert({nd.type, nd});
  }
  {
    NodeDefinition nd{NodeType::InputPosition, "Position", inputsColor, 70};
    nd.addOutput("", PinType::Vec3);
    nd.setDrawContent([](Node* node) { node->drawBaseOutput(0); });
    nd.setGenerateGlsl([](const Node* node, unsigned long outputPinId) -> std::string { return "pos"; });
    defs.insert({nd.type, nd});
  }
  return std::move(defs);
}

const std::map<NodeType, NodeDefinition> nodeDefinitions = initDefinitions();

const std::map<std::string, std::map<std::string, NodeType>> nodeListTree = {
    {
        "Surface",
        {
            {"Boolean", NodeType::SurfaceBoolean},
            {"Mix", NodeType::SurfaceMix},
            {"Box", NodeType::SurfaceCreateBox},
            {"Sphere", NodeType::SurfaceCreateSphere},
            {"Plane", NodeType::SurfaceCreatePlane},
            {"Cylinder", NodeType::SurfaceCreateCylinder},
            {"Torus", NodeType::SurfaceCreateTorus},
            {"Cone", NodeType::SurfaceCreateCone},
        },
    },
    {
        "Float",
        {
            {"Value", NodeType::Float},
            {"Code", NodeType::FloatCode},
            {"Sine", NodeType::FloatSine},
        },
    },
    {
        "Vec3",
        {
            {"Value", NodeType::Vec3},
            {"Code", NodeType::Vec3Code},
            {"Math", NodeType::Vec3Math},
            {"Translate", NodeType::Vec3Translate},
            {"Scale", NodeType::Vec3Scale},
            {"Rotate", NodeType::Vec3Rotate},
            {"Split", NodeType::Vec3Split},
            {"Combine", NodeType::Vec3Combine},
        },
    },
    {
        "Light",
        {
            {"Point", NodeType::LightPoint},
        },
    },
    {
        "Input",
        {
            {"Time", NodeType::InputTime},
            {"Position", NodeType::InputPosition},
        },
    },
};

std::vector<const float*> dataPointers;
