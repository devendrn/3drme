#include <algorithm>
#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

/* Constructors */

Pin::Pin(int id, const char* name, PinType type, PinKind kind, Node* node = nullptr) : ID(id), node(node), Name(name), Type(type), Kind(kind) {}

Node::Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) : ID(id), name(name), color(color), size(0, 0) {}

SurfaceOutputNode::SurfaceOutputNode(int id) : Node(id, "Surface Output", ImColor(200, 100, 100)) { // :) ill protect this from getting inlined by clang-format
  inputs.emplace_back(id * 10 + 1, "", PinType::Surface, PinKind::Input, this);
}

SurfaceCombineNode::SurfaceCombineNode(int id) : Node(id, "Surface Combine", ImColor(100, 150, 200)) {
  inputs.emplace_back(id * 10 + 1, "Inputs", PinType::Surface, PinKind::InputMulti, this);
  outputs.emplace_back(id * 10 + 2, "Output", PinType::Surface, PinKind::Output, this);
}

SurfaceCreateBoxNode::SurfaceCreateBoxNode(int id) : Node(id, "Surface Box", ImColor(100, 200, 100)) {
  inputs.emplace_back(id * 10 + 1, "Color", PinType::Vec3, PinKind::Input, this);
  inputs.emplace_back(id * 10 + 2, "Postion", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 3, "", PinType::Surface, PinKind::Output, this);
}

Vec3TranslateNode::Vec3TranslateNode(int id) : Node(id, "Vec3 Translate", ImColor(200, 100, 100)) {
  inputs.emplace_back(id * 10 + 1, "Input", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 2, "Output", PinType::Vec3, PinKind::Output, this);
}

Vec3ScaleNode::Vec3ScaleNode(int id) : Node(id, "Vec3 Scale", ImColor(200, 100, 100)) {
  inputs.emplace_back(id * 10 + 1, "Input", PinType::Vec3, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 2, "Output", PinType::Vec3, PinKind::Output, this);
}

InputPosNode::InputPosNode(int id) : Node(id, "Position", ImColor(200, 100, 100)) { //
  outputs.emplace_back(id * 10 + 1, "", PinType::Vec3, PinKind::Output, this);
}

InputTimeNode::InputTimeNode(int id) : Node(id, "Time", ImColor(200, 100, 100)) { //
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
    ImGui::PushItemWidth(60.0f);

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

void SurfaceCombineNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);

  Pin& i0 = inputs[0];
  drawBaseInput(i0);
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

  ImGui::DragFloat("X", &val.x);
  ImGui::DragFloat("Y", &val.y);
  ImGui::DragFloat("Z", &val.z);

  Pin& i0 = inputs[0];
  drawBaseInput(i0);
}

void Vec3ScaleNode::drawContent() {
  Pin& o0 = outputs[0];
  drawBaseOutput(o0);

  ImGui::DragFloat("X", &val.x);
  ImGui::DragFloat("Y", &val.y);
  ImGui::DragFloat("Z", &val.z);

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

std::string SurfaceCombineNode::generateGlsl() const {
  const Pin& in = inputs[0];
  if (in.pins.empty())
    return surfaceDefault;

  auto l = in.pins.size();

  std::string result = in.pins[0]->node->generateGlsl();

  if (l == 1)
    return result;

  for (int i = 1; i < l; i++) {
    result = "minSurf(" + result + "," + in.pins[i]->node->generateGlsl() + ")";
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
