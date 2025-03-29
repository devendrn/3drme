#include "ui.hpp"

#include <cfloat>
#include <format>
#include <iostream>

#include <glm/fwd.hpp>
#include <imfilebrowser.h>
#include <imgui.h>

#include "imgui_node_editor.h"
#include "node_graph.hpp"
#include "nodes.hpp"
#include "projectdata.hpp"
#include "scene.hpp"
#include "viewport.hpp"

void setupImGuiStyle() {
  // Windark style by DestroyerDarkNess from ImThemes
  ImGuiStyle& style = ImGui::GetStyle();

  style.Alpha = 1.0f;
  style.DisabledAlpha = 0.6f;
  style.WindowPadding = ImVec2(8.0f, 8.0f);
  style.WindowRounding = 8.4f;
  style.WindowBorderSize = 1.0f;
  style.WindowMinSize = ImVec2(32.0f, 32.0f);
  style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
  style.WindowMenuButtonPosition = ImGuiDir_Right;
  style.ChildRounding = 3.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupRounding = 3.0f;
  style.PopupBorderSize = 1.0f;
  style.FramePadding = ImVec2(4.0f, 3.0f);
  style.FrameRounding = 3.0f;
  style.FrameBorderSize = 1.0f;
  style.ItemSpacing = ImVec2(8.0f, 4.0f);
  style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
  style.CellPadding = ImVec2(4.0f, 2.0f);
  style.IndentSpacing = 21.0f;
  style.ColumnsMinSpacing = 6.0f;
  style.ScrollbarSize = 5.6f;
  style.ScrollbarRounding = 18.0f;
  style.GrabMinSize = 10.0f;
  style.GrabRounding = 3.0f;
  style.TabRounding = 3.0f;
  style.TabBorderSize = 0.0f;
  style.ColorButtonPosition = ImGuiDir_Right;
  style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
  style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

  style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.0f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.47f, 0.84f, 1.0f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.47f, 0.84f, 1.0f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.33f, 0.60f, 1.0f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
  style.Colors[ImGuiCol_Separator] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
  style.Colors[ImGuiCol_Tab] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_TabHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.17f, 0.17f, 0.17f, 1.0f);
  style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.47f, 0.84f, 1.0f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.33f, 0.60f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.47f, 0.84f, 1.0f);
  style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.33f, 0.60f, 1.0f);
  style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.0f);
  style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.0f);
  style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.0f);
  style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.47f, 0.84f, 1.0f);
  style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.90f);
  style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
  style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
  style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
  style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void setStyle() {
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/DroidSans.ttf", 15);
  ImGui::StyleColorsDark();
  setupImGuiStyle();
}

void updateWindowTitle(GLFWwindow* window, std::string& filepath) {
  std::string title;
  if (filepath.empty()) {
    title = "(*Unnamed)";
  } else {
    auto lastSlash = filepath.find_last_of('/');
    if (lastSlash == std::string::npos) {
      title = filepath;
    } else {
      title = filepath.substr(lastSlash + 1);
    }
  }
  title += " - Aktinomarcher";
  glfwSetWindowTitle(window, title.c_str());
};

void setupUi(GLFWwindow* window) {
  setStyle();
  std::string a;
  updateWindowTitle(window, a);
}

void reloadNodeScene(NodeEditor& nodeEditor, Shader& shader) {
  std::string surfaceCode;
  std::string skyCode;
  std::string lightsCode;
  nodeEditor.generateGlslCode(surfaceCode, skyCode, lightsCode);

  shader.resetFshSource();
  std::string& code = shader.fshEdited;

  auto line = code.find("// !sky_inline");
  code.insert(line, skyCode);
  line = code.find("// !sdf_inline", line);
  code.insert(line, surfaceCode);
  line = code.find("// !lights_inline", line);
  code.insert(line, lightsCode);

  std::cout << "[Node editor] Inline shader code: Surface\n" << surfaceCode << "\n";
  std::cout << "[Node editor] Inline shader code: Sky\n" << skyCode << "\n";
  std::cout << "[Node editor] Inline shader code: Lights\n" << lightsCode << "\n";
  shader.reloadFragment();
}

void buildUi(GLFWwindow* window, ProjectData& pd, Viewport& viewport, Scene& scene, NodeEditor& nodeEditor) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // processInput(window);

  static auto loadFileDialog = ImGui::FileBrowser();
  static auto saveFileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_EnterNewFilename);
  loadFileDialog.SetTitle("Load project file");
  loadFileDialog.SetTypeFilters({".prj"});
  saveFileDialog.SetTitle("Save project file");
  saveFileDialog.SetTypeFilters({".prj"});

  static bool optFullscreenPersistant = true;
  static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  ImGuiIO& io = ImGui::GetIO();
  if (io.KeyCtrl) {
    if (ImGui::IsKeyPressed(ImGuiKey_S)) {
      if ((io.KeyShift || !pd.hasLoadedProjectFile())) {
        saveFileDialog.Open();
      } else {
        pd.saveProjectFile(scene, viewport, nodeEditor);
      }
    } else if (ImGui::IsKeyPressed(ImGuiKey_O)) {
      loadFileDialog.Open();
    } else if (ImGui::IsKeyPressed(ImGuiKey_R)) {
      viewport.shader.reloadFshSource();
      reloadNodeScene(nodeEditor, viewport.shader);
      viewport.shader.reloadFragment();
    }
  }

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
        if (ImGui::MenuItem("Load", "Ctrl+O")) {
          loadFileDialog.Open();
        }
        if (ImGui::MenuItem("Save", "Ctrl+S", false, pd.hasLoadedProjectFile())) {
          pd.saveProjectFile(scene, viewport, nodeEditor);
        }
        if (ImGui::MenuItem("Save as", "Ctrl+Shift+S")) {
          saveFileDialog.Open();
        }
        if (ImGui::MenuItem("Exit"))
          glfwSetWindowShouldClose(window, 1);
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Project")) {
        // if (ImGui::MenuItem("Save image")) {
        //   saveFileDialog.Open();
        // }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("About")) {
        ImGui::Text("AktinoMarcher v%s", PROJECT_VERSION);
        ImGui::Separator();
        ImGui::Text(PROJECT_DESCRIPTION);
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();

      loadFileDialog.Display();
      saveFileDialog.Display();

      if (loadFileDialog.HasSelected()) {
        std::string path = loadFileDialog.GetSelected();
        pd.loadProjectFile(scene, viewport, nodeEditor, path);
        reloadNodeScene(nodeEditor, viewport.shader);
        updateWindowTitle(window, path);
        loadFileDialog.ClearSelected();
      }
      if (saveFileDialog.HasSelected()) {
        std::string path = saveFileDialog.GetSelected();
        pd.saveProjectFile(scene, viewport, nodeEditor, saveFileDialog.GetSelected());
        updateWindowTitle(window, path);
        saveFileDialog.ClearSelected();
      }
    }

    // ImGui::ShowDemoWindow();

    ImGui ::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar(1);
    {
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Add")) {
          if (ImGui::MenuItem("Box"))
            scene.addObject(Shape::BOX);
          if (ImGui::MenuItem("Sphere"))
            scene.addObject(Shape::SPHERE);
          ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
      }
      const ImVec2 p = ImGui::GetCursorScreenPos();
      const ImVec2 wsize = ImGui::GetContentRegionAvail();

      viewport.hovered = ImGui::IsWindowHovered();

      viewport.resize(static_cast<int>(wsize.x), static_cast<int>(wsize.y)); // only resizes if wsize changed
      ImGui::Image((ImTextureID)viewport.taaFramebuffer.textureID, wsize, ImVec2(0, 1), ImVec2(1, 0));

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
        ImGui::Checkbox("Toggle Orthographic mode", &viewport.camera.isOrtho);
        ImGui::SliderFloat("FOV", &viewport.camera.fov, 0.1, 1.5);
        ImGui::SliderFloat("Scale", &viewport.camera.scale, 0.05, 2.0);
      }
      if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Downscale", &viewport.downscaleFactor, 0.05, 1.0);
        ImGui::SliderInt("Iterations", &viewport.raymarchingSteps, 4, 128);
        ImGui::SliderFloat("Ray start", &viewport.raymarchingClipStart, 0.0, 4.0);
        ImGui::SliderFloat("Ray end", &viewport.raymarchingClipEnd, 0.5, 256.0);
        ImGui::SliderFloat("Pixel radius", &viewport.raymarchingPixelRadius, 0.0001, 0.01, "%.4f");
        ImGui::SliderFloat("TAAU Feedback", &viewport.taaFeedbackFactor, 0.0, 0.98);
      }
    }
    ImGui::End();

    auto& objs = scene.sceneTree;

    ImGui::Begin("Object Tree", nullptr);
    static unsigned int selected = UINT_MAX;
    {
      if (ImGui::BeginPopup("obj_menu_popup")) {
        if (ImGui::Selectable("Delete")) {
          scene.deleteObject(selected);
          scene.deselectObjects();
        }
        ImGui::EndPopup();
      }

      for (int i = 0; i < objs.size(); i++) {
        auto& obj = objs[i];
        if (ImGui::Selectable(obj.name.c_str(), selected == i)) {
          selected = i;
          scene.selectObject(i);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
          ImGui::OpenPopup("obj_menu_popup");
      }
    }
    ImGui::End();

    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoScrollbar);
    {
      if (selected < UINT_MAX && objs.size() > 0) {
        Object& active = scene.sceneTree[selected];

        ImGui::InputText("Name", active.name.data(), 16);

        if (ImGui::CollapsingHeader("Transformation", ImGuiTreeNodeFlags_DefaultOpen)) {
          const float s = 0.02f;
          ImGui::SeparatorText("Position");
          ImGui::DragFloat("X##position", &active.position.x, s);
          ImGui::DragFloat("Y##position", &active.position.y, s);
          ImGui::DragFloat("Z##position", &active.position.z, s);
          ImGui::SeparatorText("Scale");
          ImGui::DragFloat("X##scale", &active.scale.x, s);
          ImGui::DragFloat("Y##scale", &active.scale.y, s);
          ImGui::DragFloat("Z##scale", &active.scale.z, s);
          ImGui::SeparatorText("Rotation");
          ImGui::DragFloat("X##roation", &active.rotation.x, s);
          ImGui::DragFloat("Y##roation", &active.rotation.y, s);
          ImGui::DragFloat("Z##roation", &active.rotation.z, s);
        }
      }
    }
    ImGui::End();

    ImGui ::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Node Editor", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar(1);
    {
      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Add")) {
          for (const auto& [category, list] : nodeListTree) {
            if (ImGui::BeginMenu(category.c_str())) {
              for (const auto& [name, node] : list) {
                if (ImGui::MenuItem(name.c_str()))
                  nodeEditor.addNode(node);
              }
              ImGui::EndMenu();
            }
          }
          ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Refresh"))
          reloadNodeScene(nodeEditor, viewport.shader);

        const auto& shaderError = viewport.shader.getFragError();
        if (!shaderError.empty()) {
          ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Compilation Error!");
          if (ImGui::BeginItemTooltip()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", shaderError.c_str());
            ImGui::EndTooltip();
          }
        }
      }
      ImGui::EndMenuBar();

      ImVec2 cpos = ImGui::GetCursorPos();
      nodeEditor.show();

      ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0);
      ImGui::SetCursorScreenPos(cpos + ImVec2(0, 0));
      ImGui::BeginChild("SidebarOverlay", ImVec2(150, 0), ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);
      {
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::Text("Go to node:");
        if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, -FLT_MIN))) {
          for (const auto& n : nodeEditor.getNodes()) {
            auto id = n->getId();
            bool isSelected = ed::IsNodeSelected(id);

            ImGuiSelectableFlags flags = isSelected ? ImGuiSelectableFlags_Highlight : 0;
            if (ImGui::Selectable(std::format("[{}] {}", id.Get(), n->getName()).c_str(), isSelected, flags))
              nodeEditor.goToNode(id);
          }
          ImGui::EndListBox();
        }
      }
      ImGui::EndChild();
      ImGui::PopStyleVar();
    }
    ImGui::End();
  }
  ImGui::End(); // Dockspace
}
