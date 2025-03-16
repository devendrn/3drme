#ifndef NODES_H
#define NODES_H

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

enum class NodeType { //
  SurfaceCreateBox,
  SurfaceBoolean,
  SurfaceOutput,
  Vec3Translate,
  Vec3Scale,
  InputTime,
  InputPosition
};

enum class PinType { Vec3, Float, Surface };
enum class PinKind { Output, Input, InputMulti };

class Node;

class Pin {
public:
  ed::PinId ID;
  PinType Type;
  PinKind Kind;

  std::string Name;

  std::vector<Pin*> pins;
  Node* node; // change to &Node ?

  Pin(unsigned long id, const char* name, PinType type, PinKind kind, Node* node);

  void removeLink(Pin* target);
  void addLink(Pin* target);
};

class Node {
public:
  ed::NodeId ID;
  NodeType type;

  ImColor color;
  ImVec2 size;

  std::string name;

  std::vector<Pin> inputs, outputs;

  Node(unsigned long id, NodeType type, const char* name, ImColor color);
  ~Node();

  virtual void draw();
  virtual void drawContent() {};
  virtual std::string generateGlsl() const;

  bool isAncestor(Node* target) const;
};

struct Link {
  ed::LinkId ID;
  ed::PinId StartPinID, EndPinID;

  Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) : ID(id), StartPinID(startPinId), EndPinID(endPinId) {}
};

/* Surface Nodes */

class SurfaceOutputNode : public Node {
public:
  SurfaceOutputNode(unsigned long id);
  void drawContent() override;
  std::string generateGlsl() const override;
};

class SurfaceBooleanNode : public Node {
public:
  SurfaceBooleanNode(unsigned long id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  enum class BooleanType { Union, Intersection, Difference };

  float smooth = 0.0f;
  BooleanType type = BooleanType::Union;
};

class SurfaceCreateBoxNode : public Node {
public:
  SurfaceCreateBoxNode(unsigned long id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  glm::vec3 col = glm::vec3(0.0f);
};

/* Vec3 Nodes */

class Vec3TranslateNode : public Node {
public:
  Vec3TranslateNode(unsigned long id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  glm::vec3 val = glm::vec3(0.0f);
};

class Vec3ScaleNode : public Node {
public:
  Vec3ScaleNode(unsigned long id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  glm::vec3 val = glm::vec3(0.0f);
};

/* Input Nodes */

class InputPosNode : public Node {
public:
  InputPosNode(unsigned long id);
  std::string generateGlsl() const override;
  void drawContent() override;
};

class InputTimeNode : public Node {
public:
  InputTimeNode(unsigned long id);
  std::string generateGlsl() const override;
  void drawContent() override;
};

#endif
