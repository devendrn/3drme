#ifndef UI_H
#define UI_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "projectdata.hpp"
#include "viewport.hpp"

void setStyle();

void setupUi(GLFWwindow* window);

void buildUi(GLFWwindow* window, ProjectData& pd, Viewport& viewport, Scene& scene, NodeEditor& nodeEditor);

#endif
