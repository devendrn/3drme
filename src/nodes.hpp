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
  SurfaceBoolean,
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
};

struct PinDefinition {
  std::string name;
  PinType type;
  PinKind kind;
};

struct NodeDefinition {
  NodeType type;
  std::string name;
  std::vector<PinDefinition> inputs;
  std::vector<PinDefinition> outputs;
  std::vector<float> data;

  std::function<void(Node*, const std::vector<float>&)> setData;
  std::function<std::vector<float>(const Node*)> getData;
  std::function<void(Node*)> drawContent;
  std::function<std::string(const Node*, int)> generateGlsl;

  void initializeData(std::vector<float> d) { data = d; };
  void addInput(std::string name, PinType type, bool multi = false) { inputs.push_back({name, type, multi ? PinKind::InputMulti : PinKind::Input}); }
  void addOutput(std::string name, PinType type) { outputs.push_back({name, type, PinKind::Output}); }
  void setDrawContent(std::function<void(Node*)> func) { drawContent = func; }
  void setGenerateGlsl(std::function<std::string(const Node*, int)> func) { generateGlsl = func; }
};

class Node {
public:
  Node(unsigned long id, const NodeDefinition& definition);
  ~Node();

  void draw();
  void drawContent();
  std::string generateGlsl(int variant = 0) const;

  std::vector<float> getData() const;
  void setData(const std::vector<float>& data);

  bool isAncestor(Node* target) const;

  void addInputPin(const char* name, PinType type, bool multi = false);
  void addOutputPin(const char* name, PinType type);
  Pin* getPin(ed::PinId id);

  const ed::NodeId& getId() const { return id; };
  unsigned long getIdLong() const { return id.Get(); };
  unsigned long getLastId() const { return lastId; };
  NodeType getType() const { return definition.type; };
  const std::string& getName() const { return definition.name; };
  const std::vector<Pin>& getInputs() const { return inputs; };
  const std::vector<Pin>& getOutputs() const { return outputs; };

  std::vector<Pin> inputs;
  std::vector<Pin> outputs;
  std::vector<float> data;

  std::string pin0GenerateGlsl(int pinIndex, std::string defaultCode) const;

private:
  ed::NodeId id;
  unsigned long lastId;
  const NodeDefinition& definition;

  // TODO: Use these
  ImColor color;
  ImVec2 size;
};

struct Link {
  ed::LinkId id;
  ed::PinId StartPinId;
  ed::PinId EndPinId;

  Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) : id(id), StartPinId(startPinId), EndPinId(endPinId) {}
};

extern const std::map<NodeType, NodeDefinition> nodeDefinitions;

#endif
