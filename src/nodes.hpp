#ifndef NODES_H
#define NODES_H

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

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

  Pin(int id, const char* name, PinType type, PinKind kind, Node* node);

  void removeLink(Pin* target);
  void addLink(Pin* target);
};

class Node {
public:
  ed::NodeId ID;

  ImColor color;
  ImVec2 size;

  std::string name;

  std::vector<Pin> inputs;
  std::vector<Pin> outputs;

  Node(int id, const char* name, ImColor color);
  ~Node();

  virtual void draw();
  virtual void drawContent() {};
  virtual std::string generateGlsl() const;
};

struct Link {
  ed::LinkId ID;
  ed::PinId StartPinID;
  ed::PinId EndPinID;

  Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) : ID(id), StartPinID(startPinId), EndPinID(endPinId) {}
};

/* Surface Nodes */

class SurfaceOutputNode : public Node {
public:
  SurfaceOutputNode(int id);
  void drawContent() override;
  std::string generateGlsl() const override;
};

class SurfaceCombineNode : public Node {
public:
  SurfaceCombineNode(int id);
  std::string generateGlsl() const override;
  void drawContent() override;
};

class SurfaceCreateBoxNode : public Node {
public:
  SurfaceCreateBoxNode(int id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  glm::vec3 col = glm::vec3(0.0f);
};

/* Vec3 Nodes */

class Vec3TranslateNode : public Node {
public:
  Vec3TranslateNode(int id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  glm::vec3 val;
};

class Vec3ScaleNode : public Node {
public:
  Vec3ScaleNode(int id);
  std::string generateGlsl() const override;
  void drawContent() override;

private:
  glm::vec3 val;
};

/* Input Nodes */

class InputPosNode : public Node {
public:
  InputPosNode(int id);
  std::string generateGlsl() const override;
  void drawContent() override;
};

class InputTimeNode : public Node {
public:
  InputTimeNode(int id);
  std::string generateGlsl() const override;
  void drawContent() override;
};

#endif
