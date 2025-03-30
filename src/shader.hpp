#ifndef SHADER_H
#define SHADER_H

#include <string>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Shader {
public:
  unsigned int ID;
  std::string fshEdited;

  Shader(const std::string& name);

  void use() const;

  void reloadFragment();

  void reloadFshSource();
  void reloadVshSource();

  void resetFshSource();

  void setUniformInt(const std::string& name, int value) const;

  void setUniformFloat(const std::string& name, float value) const;
  void setUniformFloat(const std::string& name, float* values, int size) const;

  void setUniformVec2(const std::string& name, const glm::vec2& value) const;
  void setUniformVec3(const std::string& name, const glm::vec3& value) const;
  void setUniformVec4(const std::string& name, const glm::vec4& value) const;

  void setUniformMat3(const std::string& name, const glm::mat3& value) const;
  void setUniformMat4(const std::string& name, const glm::mat4& value) const;


  const std::string& getFragError() const;

private:
  std::string name;
  unsigned int fragmentShader;
  unsigned int vertexShader;
  std::string fsh;
  std::string vsh;
  std::string fragError;

  std::string readFile(const std::string& filePath);

  void loadShader(GLenum type, const char* code);

  void attachLinkShaders();
};

#endif
