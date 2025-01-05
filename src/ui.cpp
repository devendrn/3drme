#include "ui.hpp"

#include <imgui.h>

#include "viewport.hpp"

void processInput(GLFWwindow* window) {
  // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  //   glfwSetWindowShouldClose(window, 1);
}

void buildUi(GLFWwindow* window, Viewport* viewport) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  processInput(window);

  static bool optFullscreenPersistant = true;
  static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  if (optFullscreenPersistant) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport(); // const correctness
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  if ((dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0)
    windowFlags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("DockSpace", nullptr, windowFlags);
  ImGui::PopStyleVar(1);
  {
    ImGuiID dockspaceId = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspaceId, ImVec2(0, 0), dockspaceFlags);
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit"))
          glfwSetWindowShouldClose(window, 1);
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Project")) {
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("About")) {
        ImGui::MenuItem("AktinoMarcher v0.1");
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar(1);
    {
      const ImVec2 p = ImGui::GetCursorScreenPos();
      const ImVec2 wsize = ImGui::GetContentRegionAvail();

      viewport->hovered = ImGui::IsWindowHovered();

      viewport->resize(static_cast<int>(wsize.x), static_cast<int>(wsize.y)); // only resizes if wsize changed
      ImGui::Image((ImTextureID)viewport->taaFramebuffer.textureID, wsize, ImVec2(0, 1), ImVec2(1, 0));

      std::string fpsText(5, '\0');
      sprintf(fpsText.data(), "%.0f FPS", ImGui::GetIO().Framerate);
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      drawList->AddRectFilled(ImVec2(p.x + 5.0f, p.y + 5.0f), ImVec2(p.x + 72.0f, p.y + 20.0f), ImColor(0.0f, 0.0f, 0.0f, 0.5f));
      drawList->AddText(ImVec2(p.x + 8.0f, p.y + 5.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f), fpsText.data());
    }
    ImGui::End();

    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar);
    {
      if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Toggle Orthographic mode", &viewport->camera.isOrtho);
        ImGui::SliderFloat("FOV", &viewport->camera.fov, 0.1, 1.5);
        ImGui::SliderFloat("Scale", &viewport->camera.scale, 0.05, 2.0);
        ImGui::SliderFloat("Downscale", &viewport->downscaleFactor, 0.05, 1.0);
        ImGui::SliderInt("Ray steps", &viewport->raymarchingSteps, 4, 128);
        ImGui::SliderFloat("Clip start", &viewport->raymarchingClipStart, 0.1, 2.0);
        ImGui::SliderFloat("Clip end", &viewport->raymarchingClipEnd, 0.5, 256.0);
        ImGui::SliderFloat("Hit bias", &viewport->raymarchingHitBias, 0.005, 0.2);
      }
      if (ImGui::CollapsingHeader("TAAU", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Feedback", &viewport->taaFeedbackFactor, 0.0, 0.98);
      }
    }
    ImGui::End();

    ImGui::Begin("Object Tree", nullptr);
    static unsigned int selected = 0;
    {
      for (const auto& [key, value] : viewport->scene->sceneTree) {
        std::string label = "Object " + std::to_string(key);
        if (ImGui::Selectable(label.c_str(), selected == key))
          selected = key;
      }
    }
    ImGui::End();

    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoScrollbar);
    {
      Object& active = viewport->scene->sceneTree[selected];

      // ImGui::InputText("Name", test, 16);

      if (ImGui::CollapsingHeader("Transformation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputFloat3("Position", &active.position.x);
        ImGui::InputFloat3("Scale", &active.scale.x);
        ImGui::InputFloat3("Rotation", &active.rotation.x);
      }

      // if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
      // }
    }
    ImGui::End();
  }
  ImGui::End();
}
