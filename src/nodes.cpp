#include <algorithm>
#include <format>
#include <iostream>
#include <string>

#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

/* Base classes */

Pin::Pin(unsigned long id, const char* name, PinType type, PinKind kind, Node* node = nullptr) : ID(id), node(node), Name(name), Type(type), Kind(kind) {}

void Pin::removeLink(Pin* target) {
  auto index = std::find(pins.begin(), pins.end(), target);
  pins.erase(index);
}

void Pin::addLink(Pin* target) { pins.push_back(target); }

Node::Node(unsigned long id, NodeType type, const char* name, ImColor color = ImColor(255, 255, 255)) : ID(id), type(type), name(name), color(color), size(0, 0) { //
  std::cout << "[Node editor] Create node " << id << ": " << name << "\n";
}

Node::~Node() {
  for (Pin& start : inputs) {
    for (auto* end : start.pins)
      end->removeLink(&start);
  }
  for (Pin& start : outputs) {
    for (auto* end : start.pins)
      end->removeLink(&start);
  }
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

unsigned long Node::getPinCount() const { return inputs.size() + outputs.size(); }

/* Node constructors */

OutputNode::OutputNode(unsigned long id) : Node(id, NodeType::Output, "Output", ImColor(200, 100, 100)) {
  inputs.emplace_back(++id, "Surface", PinType::Surface, PinKind::Input, this);
  inputs.emplace_back(++id, "Sky", PinType::Vec3, PinKind::Input, this);
}

SurfaceBooleanNode::SurfaceBooleanNode(unsigned long id) : Node(id, NodeType::SurfaceBoolean, "Surface Boolean", ImColor(100, 150, 200)) {
  inputs.emplace_back(++id, "Input A", PinType::Surface, PinKind::Input, this);
  inputs.emplace_back(++id, "Input B,C...", PinType::Surface, PinKind::InputMulti, this);
  outputs.emplace_back(++id, "Output", PinType::Surface, PinKind::Output, this);
}

SurfaceCreateBaseNode::SurfaceCreateBaseNode(unsigned long id, NodeType type, std::string name, std::string funcName) : Node(id, type, name.c_str(), ImColor(100, 200, 100)), funcName(funcName) {
  inputs.emplace_back(++id, "Color", PinType::Vec3, PinKind::Input, this);
  inputs.emplace_back(++id, "Postion", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(++id, "", PinType::Surface, PinKind::Output, this);
}
SurfaceCreateBoxNode::SurfaceCreateBoxNode(unsigned long id) : SurfaceCreateBaseNode(id, NodeType::SurfaceCreateBox, "Box", "sdfBox") { //
  id = id + getPinCount();
  inputs.emplace_back(++id, "Size", PinType::Vec3, PinKind::Input, this);
  inputs.emplace_back(++id, "Roundness", PinType::Float, PinKind::Input, this);
};
SurfaceCreateSphereNode::SurfaceCreateSphereNode(unsigned long id) : SurfaceCreateBaseNode(id, NodeType::SurfaceCreateSphere, "Sphere", "sdfSphere") { //
  inputs.emplace_back(id + getPinCount() + 1, "Radius", PinType::Float, PinKind::Input, this);
}

Vec3TranslateNode::Vec3TranslateNode(unsigned long id) : Node(id, NodeType::Vec3Translate, "Vec3 Translate", ImColor(200, 100, 100)) {
  inputs.emplace_back(++id, "Input", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(++id, "Output", PinType::Vec3, PinKind::Output, this);
}

Vec3ScaleNode::Vec3ScaleNode(unsigned long id) : Node(id, NodeType::Vec3Scale, "Vec3 Scale", ImColor(200, 100, 100)) {
  inputs.emplace_back(++id, "Input", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(++id, "Output", PinType::Vec3, PinKind::Output, this);
}

InputPosNode::InputPosNode(unsigned long id) : Node(id, NodeType::InputPosition, "Position", ImColor(200, 100, 100)) { //
  outputs.emplace_back(++id, "", PinType::Vec3, PinKind::Output, this);
}

InputTimeNode::InputTimeNode(unsigned long id) : Node(id, NodeType::InputTime, "Time", ImColor(200, 100, 100)) { //
  outputs.emplace_back(++id, "", PinType::Float, PinKind::Output, this);
}

/* UI builders */

void Node::draw() {
  ed::BeginNode(ID);
  {
    // TODO: Header color, centering
    ImGui::PushID(&ID);
    ImGui::PushItemWidth(102);

    ImGui::Text("%s", name.c_str());
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
  float textWidth = ImGui::CalcTextSize(pin.Name.c_str()).x;
  ImGui::Dummy(ImVec2(70.0f - textWidth, 8.0f));
  ImGui::SameLine();
  ed::BeginPin(pin.ID, ed::PinKind::Output);
  {
    ImGui::Text("%s", pin.Name.c_str());
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(16, 16));
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
    // ed::PinPivotSize(ImVec2(0, 0));
    ImColor pinColor = getPinColor(pin.Type);
    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin() + ImVec2(3, 3), ImGui::GetItemRectMin() + ImVec2(13, 13), pinColor, 6);
  }
  ed::EndPin();
}

void drawBaseInput(Pin& pin) {
  ed::BeginPin(pin.ID, ed::PinKind::Input);
  {
    float extraHeight = pin.Kind == PinKind::InputMulti ? 2 : 0;
    ImGui::Dummy(ImVec2(16, 16));
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ImColor pinColor = getPinColor(pin.Type);
    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin() + ImVec2(3, 3 - extraHeight), ImGui::GetItemRectMin() + ImVec2(13 - extraHeight, 13 + extraHeight), pinColor, 6);
    ImGui::SameLine();
    ImGui::Text("%s", pin.Name.c_str());
  }
  ed::EndPin();
}

void OutputNode::drawContent() {
  drawBaseInput(inputs[0]);
  drawBaseInput(inputs[1]);
}

void SurfaceBooleanNode::drawContent() {
  drawBaseOutput(outputs[0]);

  ImVec2 size = ImVec2(10, 14);
  ImGui::Dummy(ImVec2(20, 5));
  ImGui::SameLine();
  if (ImGui::Selectable("U", type == BooleanType::Union, 0, size))
    type = BooleanType::Union;
  ImGui::SameLine();
  if (ImGui::Selectable("D", type == BooleanType::Difference, 0, size))
    type = BooleanType::Difference;
  ImGui::SameLine();
  if (ImGui::Selectable("I", type == BooleanType::Intersection, 0, size))
    type = BooleanType::Intersection;
  ImGui::DragFloat("##smooth", &smooth, 0.01f, 0.0, 1.0f);

  drawBaseInput(inputs[0]);
  drawBaseInput(inputs[1]);
}

void SurfaceCreateBaseNode::drawContent() {
  drawBaseOutput(outputs[0]);

  Pin& i0 = inputs[0];
  Pin& i1 = inputs[1];
  drawBaseInput(i0);
  if (i0.pins.empty()) {
    ImGui::ColorEdit3("##col", &col.x);
    ImGui::Dummy(ImVec2(0, 4));
  }
  drawBaseInput(i1);
  if (i1.pins.empty()) {
    ImGui::DragFloat3("##pos", &pos.x);
    ImGui::Dummy(ImVec2(0, 4));
  }
}
void SurfaceCreateBoxNode::drawContent() {
  SurfaceCreateBaseNode::drawContent();
  Pin& i2 = inputs[2];
  Pin& i3 = inputs[3];
  drawBaseInput(i2);
  if (i2.pins.empty()) {
    ImGui::DragFloat3("##bou", &bounds.x);
    ImGui::Dummy(ImVec2(0, 4));
  }
  drawBaseInput(i3);
  if (i3.pins.empty()) {
    ImGui::DragFloat("##rou", &roundness);
  }
}
void SurfaceCreateSphereNode::drawContent() {
  SurfaceCreateBaseNode::drawContent();
  Pin& i2 = inputs[2];
  drawBaseInput(i2);
  if (i2.pins.empty()) {
    ImGui::DragFloat("##rad", &radius);
  }
}

void Vec3TranslateNode::drawContent() {
  drawBaseOutput(outputs[0]);

  ImGui::DragFloat("##x", &val.x);
  ImGui::DragFloat("##y", &val.y);
  ImGui::DragFloat("##z", &val.z);

  Pin& i0 = inputs[0];
  drawBaseInput(i0);
}

void Vec3ScaleNode::drawContent() {
  drawBaseOutput(outputs[0]);

  ImGui::DragFloat("##x", &val.x);
  ImGui::DragFloat("##y", &val.y);
  ImGui::DragFloat("##z", &val.z);

  Pin& i0 = inputs[0];
  drawBaseInput(i0);
}

void InputPosNode::drawContent() { //
  drawBaseOutput(outputs[0]);
}

void InputTimeNode::drawContent() { //
  drawBaseOutput(outputs[0]);
}

/* Data loading/saving */

void Node::setData(const std::vector<float>& data) {}
std::vector<float> Node::getData() const { return {}; }

std::vector<float> SurfaceBooleanNode::getData() const { return {smooth, static_cast<float>(type)}; }
void SurfaceBooleanNode::setData(const std::vector<float>& data) {
  if (data.size() >= 2) {
    smooth = data[0];
    type = static_cast<BooleanType>(static_cast<int>(data[1]));
  }
}

std::vector<float> SurfaceCreateBaseNode::getData() const { return {col.x, col.y, col.z, pos.x, pos.y, pos.z}; }
void SurfaceCreateBaseNode::setData(const std::vector<float>& data) {
  if (data.size() >= 6) {
    col.x = data[0];
    col.y = data[1];
    col.z = data[2];
    pos.x = data[3];
    pos.y = data[4];
    pos.z = data[5];
  }
}
std::vector<float> SurfaceCreateBoxNode::getData() const {
  auto data = SurfaceCreateBaseNode::getData();
  data.push_back(bounds.x); // 6
  data.push_back(bounds.y);
  data.push_back(bounds.z);
  data.push_back(roundness); // 9
  return data;
}
void SurfaceCreateBoxNode::setData(const std::vector<float>& data) {
  SurfaceCreateBaseNode::setData(data);
  if (data.size() >= 9) {
    bounds.x = data[6];
    bounds.y = data[7];
    bounds.z = data[8];
    roundness = data[9];
  }
}
std::vector<float> SurfaceCreateSphereNode::getData() const {
  auto data = SurfaceCreateBaseNode::getData();
  data.push_back(radius); // 6
  return data;
}
void SurfaceCreateSphereNode::setData(const std::vector<float>& data) {
  SurfaceCreateBaseNode::setData(data);
  if (data.size() >= 6) {
    radius = data[6];
  }
}

std::vector<float> Vec3TranslateNode::getData() const { return {val.x, val.y, val.z}; }
void Vec3TranslateNode::setData(const std::vector<float>& data) {
  if (data.size() >= 3) {
    val.x = data[0];
    val.y = data[1];
    val.z = data[2];
  }
}

std::vector<float> Vec3ScaleNode::getData() const { return {val.x, val.y, val.z}; }
void Vec3ScaleNode::setData(const std::vector<float>& data) {
  if (data.size() >= 3) {
    val.x = data[0];
    val.y = data[1];
    val.z = data[2];
  }
}

/* Graph parsing */

const char* surfaceDefault = "Surface(FLOAT_MAX,vec3(0),0.0)";

std::string vec3ToString(glm::vec3 val) { return std::format("vec3({},{},{})", val.x, val.y, val.z); }

std::string Node::generateGlsl() const { return ""; }

std::string OutputNode::generateGlsl() const {
  const Pin& i0 = inputs[0];
  if (i0.pins.empty())
    return "";
  return std::format("s={};", i0.pins[0]->node->generateGlsl());
}

std::string OutputNode::generateSkyGlsl() const {
  const Pin& i1 = inputs[1];
  if (i1.pins.empty())
    return "";
  return std::format("s={};", i1.pins[0]->node->generateGlsl());
}

std::string SurfaceBooleanNode::generateGlsl() const {
  const Pin& i0 = inputs[0];
  const Pin& i1 = inputs[1];

  for (const auto& p : i1.pins)
    std::cout << p->Name << "\n";

  if (i0.pins.empty())
    return surfaceDefault;

  std::string result = i0.pins[0]->node->generateGlsl();

  if (i1.pins.empty())
    return result;

  std::string func;
  std::string end = ")";
  if (type == BooleanType::Union) {
    func = "uSurf";
    if (smooth > 0.0)
      end = "," + std::to_string(smooth) + ")";
  } else if (type == BooleanType::Difference)
    func = "dSurf";
  else if (type == BooleanType::Intersection)
    func = "iSurf";

  auto l = i1.pins.size();
  for (int i = 0; i < l; i++)
    result = std::format("{}({},{}{}", func, result, i1.pins[i]->node->generateGlsl(), end);

  return result;
}

std::string SurfaceCreateBaseNode::generateGlsl() const {
  std::string posCode = "pos+" + vec3ToString(pos);
  std::string colCode = vec3ToString(col);
  if (!inputs[0].pins.empty())
    colCode = inputs[0].pins[0]->node->generateGlsl();
  if (!inputs[1].pins.empty())
    posCode = inputs[1].pins[0]->node->generateGlsl();
  return std::format("Surface({}({}{}),{},0.0)", funcName, posCode, args(), colCode);
}
std::string SurfaceCreateBoxNode::args() const {
  std::string s = ",";

  if (inputs[2].pins.empty())
    s += vec3ToString(bounds);
  else
    s += inputs[2].pins[0]->node->generateGlsl();

  if (inputs[3].pins.empty())
    s += std::format(",{:.3f}", roundness);
  else
    s += std::format(",{}", inputs[3].pins[0]->node->generateGlsl());

  return s;
}
std::string SurfaceCreateSphereNode::args() const {
  if (inputs[2].pins.empty())
    return std::format(",{:.3f}", radius);
  return std::format(",{}", inputs[2].pins[0]->node->generateGlsl());
}

std::string Vec3TranslateNode::generateGlsl() const {
  std::string b = vec3ToString(val);
  if (inputs[0].pins.empty())
    return b;
  return std::format("({}+{})", inputs[0].pins[0]->node->generateGlsl(), b);
}

std::string Vec3ScaleNode::generateGlsl() const {
  std::string b = vec3ToString(val);
  if (inputs[0].pins.empty())
    return b;
  return std::format("({}*{})", inputs[0].pins[0]->node->generateGlsl(), b);
}

std::string InputPosNode::generateGlsl() const { //
  return "pos";
}

std::string InputTimeNode::generateGlsl() const { //
  return "t";
}
