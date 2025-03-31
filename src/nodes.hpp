#ifndef NODES_H
#define NODES_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

enum class NodeType { //
  Output,
  SurfaceCreateBox,
  SurfaceCreateSphere,
  SurfaceCreateCylinder,
  SurfaceCreateTorus,
  SurfaceCreateCone,
  SurfaceCreatePlane,
  SurfaceBoolean,
  SurfaceMix,
  Float,
  FloatCode,
  FloatSine,
  Vec3,
  Vec3Code,
  Vec3Math,
  Vec3Translate,
  Vec3Scale,
  Vec3Rotate,
  Vec3Split,
  Vec3Combine,
  LightPoint,
  LightDirectional,
  InputTime,
  InputPosition
};

enum class PinType { Vec3, Float, Surface, Light };
enum class PinKind { Output, Input, InputMulti };

class Node;

class Pin {
public:
  ed::PinId id;
  PinType type;
  PinKind kind;

  std::string name;

  std::vector<Pin*> pins;
  Node* node; // change to &Node ?

  Pin(unsigned long id, const char* name, PinType type, PinKind kind, Node* node);

  void removeLink(const Pin* target);
  void addLink(Pin* target);
  void clearLinks();

  std::string generateGlsl() const; // only for output pin
};

ImColor getPinColor(PinType type);

struct PinDefinition {
  std::string name;
  PinType type;
  PinKind kind;
};

struct NodeDefinition {
  NodeType type;
  std::string name;
  ImColor color;
  float width = 160.0f;
  std::vector<PinDefinition> inputs;
  std::vector<PinDefinition> outputs;
  std::vector<float> data;

  std::function<void(Node*, const std::vector<float>&)> setData;
  std::function<std::vector<float>(const Node*)> getData;
  std::function<void(Node*)> drawContent;
  std::function<std::string(const Node*, unsigned long)> generateGlsl;

  void initializeData(std::vector<float> d) { data = d; };
  void addInput(std::string name, PinType type, bool multi = false) { inputs.push_back({name, type, multi ? PinKind::InputMulti : PinKind::Input}); }
  void addOutput(std::string name, PinType type) { outputs.push_back({name, type, PinKind::Output}); }
  void setDrawContent(std::function<void(Node*)> func) { drawContent = func; }
  void setGenerateGlsl(std::function<std::string(const Node*, unsigned long)> func) { generateGlsl = func; }
};

class Node {
public:
  std::vector<Pin> inputs;
  std::vector<Pin> outputs;
  std::vector<float> data;
  std::string code; // only used by custom code nodes

  Node(unsigned long id, const NodeDefinition& definition);
  ~Node();

  void draw();
  void drawContent();
  std::string generateGlsl(unsigned long outputPinId) const;
  std::vector<float> getData() const;
  void setData(const std::vector<float>& data);
  bool isAncestor(Node* target) const;
  void addInputPin(const char* name, PinType type, bool multi = false);
  void addOutputPin(const char* name, PinType type);
  Pin* getPin(ed::PinId id);
  void drawBaseOutput(int index);
  void drawBaseInput(int index, std::function<void()> inner = [] {});
  std::string pin0GenerateGlsl(int pinIndex, std::string defaultCode) const;

  const ed::NodeId& getId() const;
  unsigned long getIdLong() const;
  unsigned long getLastId() const;
  NodeType getType() const;
  const std::string& getName() const;
  const std::vector<Pin>& getInputs() const;
  const std::vector<Pin>& getOutputs() const;

private:
  ed::NodeId id;
  unsigned long lastId;
  const NodeDefinition& definition;
};

struct Link {
  ed::LinkId id;
  ed::PinId StartPinId;
  ed::PinId EndPinId;

  Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) : id(id), StartPinId(startPinId), EndPinId(endPinId) {}
};

extern const std::map<NodeType, NodeDefinition> nodeDefinitions;

extern const std::map<std::string, std::map<std::string, NodeType>> nodeListTree;

extern std::vector<const float*> dataPointers;

#endif
