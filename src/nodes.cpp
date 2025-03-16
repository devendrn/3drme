#include <algorithm>
#include <imgui.h>
#include <imgui_node_editor.h>
#include <string>

#include "nodes.hpp"

/* Constructors */

Pin::Pin(unsigned long id, const char* name, PinType type, PinKind kind, Node* node = nullptr) : ID(id), node(node), Name(name), Type(type), Kind(kind) {}

Node::Node(unsigned long id, NodeType type, const char* name, ImColor color = ImColor(255, 255, 255)) : ID(id), type(type), name(name), color(color), size(0, 0) {}

SurfaceOutputNode::SurfaceOutputNode(unsigned long id) : Node(id, NodeType::SurfaceOutput, "Surface Output", ImColor(200, 100, 100)) { // :) ill protect this from getting inlined by clang-format
  inputs.emplace_back(id * 10 + 1, "", PinType::Surface, PinKind::Input, this);
}

SurfaceBooleanNode::SurfaceBooleanNode(unsigned long id) : Node(id, NodeType::SurfaceBoolean, "Surface Boolean", ImColor(100, 150, 200)) {
  inputs.emplace_back(id * 10 + 1, "Input A", PinType::Surface, PinKind::Input, this);
  inputs.emplace_back(id * 10 + 2, "Input B,C...", PinType::Surface, PinKind::InputMulti, this);
  outputs.emplace_back(id * 10 + 3, "Output", PinType::Surface, PinKind::Output, this);
}

SurfaceCreateBoxNode::SurfaceCreateBoxNode(unsigned long id) : Node(id, NodeType::SurfaceCreateBox, "Surface Box", ImColor(100, 200, 100)) {
  inputs.emplace_back(id * 10 + 1, "Color", PinType::Vec3, PinKind::Input, this);
  inputs.emplace_back(id * 10 + 2, "Postion", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 3, "", PinType::Surface, PinKind::Output, this);
}

Vec3TranslateNode::Vec3TranslateNode(unsigned long id) : Node(id, NodeType::Vec3Translate, "Vec3 Translate", ImColor(200, 100, 100)) {
  inputs.emplace_back(id * 10 + 1, "Input", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 2, "Output", PinType::Vec3, PinKind::Output, this);
}

Vec3ScaleNode::Vec3ScaleNode(unsigned long id) : Node(id, NodeType::Vec3Scale, "Vec3 Scale", ImColor(200, 100, 100)) {
  inputs.emplace_back(id * 10 + 1, "Input", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 2, "Output", PinType::Vec3, PinKind::Output, this);
}

InputPosNode::InputPosNode(unsigned long id) : Node(id, NodeType::InputPosition, "Position", ImColor(200, 100, 100)) { //
  outputs.emplace_back(id * 10 + 1, "", PinType::Vec3, PinKind::Output, this);
}

InputTimeNode::InputTimeNode(unsigned long id) : Node(id, NodeType::InputTime, "Time", ImColor(200, 100, 100)) { //
  outputs.emplace_back(id * 10 + 1, "", PinType::Float, PinKind::Output, this);
}

/* Destructors */

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

/* Manage Links */

void Pin::removeLink(Pin* target) {
  auto index = std::find(pins.begin(), pins.end(), target);
  if (index != pins.end())
    pins.erase(index);
}

void Pin::addLink(Pin* target) { pins.push_back(target); }

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
  default:
    return ImColor(1.0f, 1.0f, 1.0f);
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

void SurfaceOutputNode::drawContent() {
  Pin& i0 = inputs[0];
  drawBaseInput(i0);
}

void SurfaceBooleanNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);

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

  Pin& i0 = inputs[0];
  Pin& i1 = inputs[1];
  drawBaseInput(i0);
  drawBaseInput(i1);
}

void SurfaceCreateBoxNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);

  Pin& i0 = inputs[0];
  Pin& i1 = inputs[1];
  drawBaseInput(i0);
  drawBaseInput(i1);
}

void Vec3TranslateNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);

  ImGui::DragFloat("##x", &val.x);
  ImGui::DragFloat("##y", &val.y);
  ImGui::DragFloat("##z", &val.z);

  Pin& i0 = inputs[0];
  drawBaseInput(i0);
}

void Vec3ScaleNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);

  ImGui::DragFloat("##x", &val.x);
  ImGui::DragFloat("##y", &val.y);
  ImGui::DragFloat("##z", &val.z);

  Pin& i0 = inputs[0];
  drawBaseInput(i0);
}

void InputPosNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);
}

void InputTimeNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);
}

/* Graph parsing */

const char* surfaceDefault = "Surface(FLOAT_MAX,vec3(0.0),0.0)";

std::string vec3ToString(glm::vec3 val) { return "vec3(" + std::to_string(val.x) + "," + std::to_string(val.y) + "," + std::to_string(val.z) + ")"; }

std::string Node::generateGlsl() const { return ""; }

std::string SurfaceOutputNode::generateGlsl() const {
  if (!inputs[0].pins.empty())
    return "s = " + inputs[0].pins[0]->node->generateGlsl() + ";";
  return "";
}

std::string SurfaceBooleanNode::generateGlsl() const {
  const Pin& i0 = inputs[0];
  const Pin& i1 = inputs[1];
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
  for (int i = 0; i < l; i++) {
    result = func + "(" + result + "," + i1.pins[i]->node->generateGlsl() + end;
  }

  return result;
}

std::string SurfaceCreateBoxNode::generateGlsl() const {
  std::string posCode = "pos";
  std::string colCode = vec3ToString(col);
  if (!inputs[0].pins.empty())
    colCode = inputs[0].pins[0]->node->generateGlsl();
  if (!inputs[1].pins.empty())
    posCode = inputs[1].pins[0]->node->generateGlsl();
  return "Surface(sdfBox(" + posCode + ")," + colCode + ",0.0)";
}

std::string Vec3TranslateNode::generateGlsl() const {
  std::string b = vec3ToString(val);
  if (!inputs[0].pins.empty()) {
    return "(" + inputs[0].pins[0]->node->generateGlsl() + "+" + b + ")";
  }
  return b;
}

std::string Vec3ScaleNode::generateGlsl() const {
  if (!inputs[0].pins.empty()) {
    std::string b = vec3ToString(val);
    return "(" + inputs[0].pins[0]->node->generateGlsl() + "*" + b + ")";
  }
  return "vec3(0.0)";
}

std::string InputPosNode::generateGlsl() const { //
  return "pos";
}

std::string InputTimeNode::generateGlsl() const { //
  return "t";
}
