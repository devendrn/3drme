#include <imgui.h>
#include <imgui_node_editor.h>

#include "nodes.hpp"

/* Constructors */

Pin::Pin(int id, const char* name, PinType type, PinKind kind, Node* node = nullptr) : ID(id), pin(nullptr), node(node), Name(name), Type(type), Kind(kind) {}

Node::Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) : ID(id), name(name), color(color), size(0, 0) {}

SurfaceOutputNode::SurfaceOutputNode(int id) : Node(id, "Surface Output", ImColor(200, 100, 100)) { // :) ill protect this from getting inlined by clang-format
  inputs.emplace_back(id * 10 + 1, "", PinType::Surface, PinKind::Input, this);
}

SurfaceCombineNode::SurfaceCombineNode(int id) : Node(id, "Surface Combine", ImColor(100, 150, 200)) {
  inputs.emplace_back(id * 10 + 1, "A", PinType::Surface, PinKind::Input, this);
  inputs.emplace_back(id * 10 + 2, "B", PinType::Surface, PinKind::Input, this);
  outputs.emplace_back(id * 10 + 3, "", PinType::Surface, PinKind::Output, this);
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
  for (auto i : inputs) {
    if (i.pin != nullptr) {
      i.pin->pin = nullptr;
      // i.pin = nullptr;
    }
  }
  for (auto i : outputs) {
    if (i.pin != nullptr) {
      i.pin->pin = nullptr;
      // i.pin = nullptr;
    }
  }
}

/* UI builders */

void Node::draw() {
  // TODO: Header color, centering
  ImGui::PushID(static_cast<int>(ID.Get()));
  ImGui::PushItemWidth(60.0f);

  ImGui::Text("%s", name.c_str());
  drawContent();

  ImGui::PopItemWidth();
  ImGui::PopID();
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
  ImGui::Dummy(ImVec2(50.0f - textWidth, 8.0f));
  ImGui::SameLine();
  ImGui::Text("%s", pin.Name.c_str());
  ImGui::SameLine();
  ed::BeginPin(pin.ID, ed::PinKind::Output);
  {
    ImGui::Dummy(ImVec2(16, 16));              // Make space for the circle.
    ed::PinPivotAlignment(ImVec2(1.0f, 0.5f)); // Align pin to the right
    // ed::PinPivotSize(ImVec2(0, 0));            // hide the default pin rendering
    ImColor pinColor = getPinColor(pin.Type);
    ImGui::GetWindowDrawList()->AddCircleFilled(ImGui::GetItemRectMin() + ImVec2(8, 8), 6, pinColor);
  }
  ed::EndPin();
}

void drawBaseInput(Pin& pin) {
  ed::BeginPin(pin.ID, ed::PinKind::Input);
  {
    ImGui::Dummy(ImVec2(16, 16));
    ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
    ImColor pinColor = getPinColor(pin.Type);
    ImGui::GetWindowDrawList()->AddCircleFilled(ImGui::GetItemRectMin() + ImVec2(8, 8), 6, pinColor);
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
  Pin* in = inputs[0].pin;
  if (in != nullptr) {
    if (in->node != nullptr)
      return "s = " + in->node->generateGlsl() + ";";
  }
  return "";
}

std::string SurfaceCombineNode::generateGlsl() const {
  std::string a = surfaceDefault;
  std::string b = surfaceDefault;
  if (inputs[0].pin != nullptr) {
    a = inputs[0].pin->node->generateGlsl();
  }
  if (inputs[1].pin != nullptr) {
    b = inputs[1].pin->node->generateGlsl();
  }
  return "minSurf(" + a + "," + b + ")";
}

std::string SurfaceCreateBoxNode::generateGlsl() const {
  std::string posCode = "pos";
  std::string colCode = vec3ToString(col);
  if (inputs[0].pin != nullptr) { // color
    colCode = inputs[0].pin->node->generateGlsl();
  }
  if (inputs[1].pin != nullptr) { // pos
    posCode = inputs[1].pin->node->generateGlsl();
  }
  return "Surface(sdfBox(" + posCode + ")," + colCode + ",0.0)";
}

std::string Vec3TranslateNode::generateGlsl() const {
  std::string b = vec3ToString(val);
  if (inputs[0].pin != nullptr) {
    return "(" + inputs[0].pin->node->generateGlsl() + "+" + b + ")";
  }
  return b;
}

std::string Vec3ScaleNode::generateGlsl() const {
  if (inputs[0].pin != nullptr) {
    std::string b = vec3ToString(val);
    return "(" + inputs[0].pin->node->generateGlsl() + "*" + b + ")";
  }
  return "vec3(0.0)";
}

std::string InputPosNode::generateGlsl() const { //
  return "pos";
}

std::string InputTimeNode::generateGlsl() const { //
  return "t";
}
