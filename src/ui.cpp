#include "ui.hpp"

#include <imgui.h>

#include "scene.hpp"
#include "viewport.hpp"

void setStyle() {
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/DroidSans.ttf", 15);

  ImGuiStyle& style = ImGui::GetStyle();

  // palette
  ImVec4 black = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  ImVec4 white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  ImVec4 grey = ImVec4(0.60f, 0.60f, 0.60f, 0.35f);
  ImVec4 dark = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
  ImVec4 darkgrey = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
  ImVec4 lightgrey = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

  style.Colors[ImGuiCol_Text] = white;
  style.Colors[ImGuiCol_TextDisabled] = grey;
  style.Colors[ImGuiCol_WindowBg] = dark;
  style.Colors[ImGuiCol_ChildBg] = dark;
  style.Colors[ImGuiCol_PopupBg] = dark;
  style.Colors[ImGuiCol_Border] = grey;
  style.Colors[ImGuiCol_BorderShadow] = black;
  style.Colors[ImGuiCol_FrameBg] = darkgrey;
  style.Colors[ImGuiCol_FrameBgHovered] = grey;
  style.Colors[ImGuiCol_FrameBgActive] = grey;
  style.Colors[ImGuiCol_TitleBg] = darkgrey;
  style.Colors[ImGuiCol_TitleBgActive] = darkgrey;
  style.Colors[ImGuiCol_TitleBgCollapsed] = darkgrey;
  style.Colors[ImGuiCol_MenuBarBg] = darkgrey;
  style.Colors[ImGuiCol_ScrollbarBg] = darkgrey;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = grey;
  style.Colors[ImGuiCol_ScrollbarGrabActive] = grey;
  style.Colors[ImGuiCol_CheckMark] = lightgrey;
  style.Colors[ImGuiCol_SliderGrab] = lightgrey;
  style.Colors[ImGuiCol_SliderGrabActive] = white;
  style.Colors[ImGuiCol_Button] = darkgrey;
  style.Colors[ImGuiCol_ButtonHovered] = grey;
  style.Colors[ImGuiCol_ButtonActive] = darkgrey;
  style.Colors[ImGuiCol_Header] = darkgrey;
  style.Colors[ImGuiCol_HeaderHovered] = grey;
  style.Colors[ImGuiCol_HeaderActive] = grey;
  style.Colors[ImGuiCol_Separator] = grey;
  style.Colors[ImGuiCol_SeparatorHovered] = grey;
  style.Colors[ImGuiCol_SeparatorActive] = grey;
  style.Colors[ImGuiCol_ResizeGrip] = darkgrey;
  style.Colors[ImGuiCol_ResizeGripHovered] = grey;
  style.Colors[ImGuiCol_ResizeGripActive] = grey;
  style.Colors[ImGuiCol_Tab] = darkgrey;
  style.Colors[ImGuiCol_TabHovered] = grey;
  style.Colors[ImGuiCol_TabActive] = grey;
  style.Colors[ImGuiCol_TabUnfocused] = grey;
  style.Colors[ImGuiCol_TabUnfocused] = grey;
  style.Colors[ImGuiCol_TabUnfocusedActive] = grey;
  style.Colors[ImGuiCol_DockingPreview] = grey;
  style.Colors[ImGuiCol_DockingEmptyBg] = grey;
  style.Colors[ImGuiCol_PlotLines] = white;
  style.Colors[ImGuiCol_PlotLinesHovered] = grey;
  style.Colors[ImGuiCol_PlotHistogram] = white;
  style.Colors[ImGuiCol_PlotHistogramHovered] = grey;
  style.Colors[ImGuiCol_TableHeaderBg] = dark;
  style.Colors[ImGuiCol_TableBorderStrong] = darkgrey;
  style.Colors[ImGuiCol_TableBorderLight] = grey;
  style.Colors[ImGuiCol_TableRowBg] = black;
  style.Colors[ImGuiCol_TableRowBgAlt] = white;
  style.Colors[ImGuiCol_TextSelectedBg] = darkgrey;
  style.Colors[ImGuiCol_DragDropTarget] = darkgrey;
  style.Colors[ImGuiCol_NavHighlight] = grey;
  style.Colors[ImGuiCol_NavWindowingHighlight] = grey;
  style.Colors[ImGuiCol_NavWindowingDimBg] = grey;
  style.Colors[ImGuiCol_ModalWindowDimBg] = grey;

  // Style
  // style.WindowPadding = ImVec2(8.0f, 8.0f);
  // style.FramePadding = ImVec2(3.00f, 3.00f);
  // style.CellPadding = ImVec2(0.00f, 5.00f);
  // style.ItemSpacing = ImVec2(6.00f, 6.00f);
  // style.ItemInnerSpacing = ImVec2(5.00f, 5.00f);
  // style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
  // style.IndentSpacing = 15;
  // style.ScrollbarSize = 18;

  // style.WindowBorderSize = 0;
  // style.ChildBorderSize = 0;
  // style.PopupBorderSize = 0;
  // style.FrameBorderSize = 0;
  // style.TabBorderSize = 0;

  style.GrabMinSize = 5;

  style.FrameRounding = 0;
  style.WindowRounding = 0;
  style.ChildRounding = 0;
  style.PopupRounding = 0;
  style.ScrollbarRounding = 0;
  style.GrabRounding = 0;
  style.TabRounding = 0;

  style.LogSliderDeadzone = 0;

  style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
}

/*
void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}*/

void buildUi(GLFWwindow* window, Viewport* viewport, Scene* scene) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // processInput(window);

  static bool optFullscreenPersistant = true;
  static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  if (optFullscreenPersistant) {
    const ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imguiViewport->WorkPos);
    ImGui::SetNextWindowSize(imguiViewport->WorkSize);
    ImGui::SetNextWindowViewport(imguiViewport->ID);
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

    // ImGui::ShowDemoWindow();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar(1);
    {
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Add")) {
          if (ImGui::MenuItem("Box"))
            scene->addObject(Shape::BOX);
          if (ImGui::MenuItem("Sphere"))
            scene->addObject(Shape::SPHERE);
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
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

    ImGui::Begin("Scene", nullptr);
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

    auto& objs = scene->sceneTree;

    ImGui::Begin("Object Tree", nullptr);
    static unsigned int selected = UINT_MAX;
    {
      if (ImGui::BeginPopup("obj_menu_popup")) {
        if (ImGui::Selectable("Delete")) {
          scene->deleteObject(selected);
          scene->deselectObjects();
        }
        ImGui::EndPopup();
      }

      for (int i = 0; i < objs.size(); i++) {
        auto& obj = objs[i];
        if (ImGui::Selectable(obj.name.c_str(), selected == i)) {
          selected = i;
          scene->selectObject(i);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
          ImGui::OpenPopup("obj_menu_popup");
      }
    }
    ImGui::End();

    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoScrollbar);
    {
      if (selected < UINT_MAX && objs.size() > 0) {
        Object& active = scene->sceneTree[selected];

        ImGui::InputText("Name", active.name.data(), 16);

        if (ImGui::CollapsingHeader("Transformation", ImGuiTreeNodeFlags_DefaultOpen)) {
          ImGui::InputFloat3("Position", &active.position.x);
          ImGui::InputFloat3("Scale", &active.scale.x);
          ImGui::InputFloat3("Rotation", &active.rotation.x);
        }
        // if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        // }
      }
    }
    ImGui::End();
  }
  ImGui::End();
}
