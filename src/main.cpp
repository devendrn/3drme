#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ui.hpp"
#include "viewport.hpp"

GLFWwindow* initializeWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1024, 512, "AktinoMarcher", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return nullptr;
  }

  glfwMakeContextCurrent(window);

  // glfwSwapInterval(0); // disable vsync

  if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return nullptr;
  }

  return window;
}

void initializeImGui(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad | ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  setStyle();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 430");
}

int main() {
  GLFWwindow* window = initializeWindow();
  if (window == nullptr)
    return -1;

  auto scene = Scene();

  Viewport viewport(window, &scene);

  initializeImGui(window);

  while (glfwWindowShouldClose(window) == 0) {
    buildUi(window, &viewport, &scene);

    viewport.render();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return 0;
}
