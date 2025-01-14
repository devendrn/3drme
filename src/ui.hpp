#ifndef UI_H
#define UI_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "viewport.hpp"

void setStyle();

void buildUi(GLFWwindow* window, Viewport* viewport, Scene* scene);

#endif
