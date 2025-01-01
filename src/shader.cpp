#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>


Shader::Shader(const std::string& name) {
  std::string vertexShaderSrc = readFile("shaders/" + name + ".vsh");
  std::string fragmentShaderSrc = readFile("shaders/" + name + ".fsh");

  unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
  unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

  attachLinkShaders(vertexShader, fragmentShader);
}

void Shader::use() const { glUseProgram(ID); }

void Shader::setUniformInt(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setUniformFloat(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setUniformVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setUniformVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setUniformVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setUniformMat3(const std::string& name, const glm::mat3& value) const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]); }
void Shader::setUniformMat4(const std::string& name, const glm::mat4& value) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]); }

std::string Shader::readFile(const std::string& filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open shader file: " + filePath);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  return buffer.str();
}

unsigned int Shader::compileShader(GLenum type, const std::string& source) {
  const char* src = source.c_str();

  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int success;
  char infoLog[1024];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == 0) {
    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
    std::cerr << "ERROR::SHADER::" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  return shader;
}

void Shader::attachLinkShaders(unsigned int vertexShader, unsigned int fragmentShader) {
  ID = glCreateProgram();
  glAttachShader(ID, vertexShader);
  glAttachShader(ID, fragmentShader);
  glLinkProgram(ID);

  int success;
  char infoLog[1024];
  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (success == 0) {
    glGetProgramInfoLog(ID, 1024, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}
