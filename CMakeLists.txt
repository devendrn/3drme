cmake_minimum_required(VERSION 3.17)

project(3d-raymarched-engine LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm 0.9 REQUIRED)

add_subdirectory(lib/glad)
add_subdirectory(lib/imgui)
add_subdirectory(lib/imgui-node-editor)
include_directories(lib/cereal/include)
include_directories(lib/imbrowser/include)

add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/ui.cpp
  src/scene.cpp
  src/viewport.cpp
  src/camera.cpp
  src/shader.cpp
  src/projectdata.cpp
  src/nodes.cpp
  src/node_graph.cpp
)

# FIXME: Use proper directory structure
install(DIRECTORY assets DESTINATION bin)
install(FILES src/shaders/*.fsh src/shaders/*.vsh DESTINATION bin/shaders)

target_link_libraries(${PROJECT_NAME} imgui glfw OpenGL::GL glm::glm glad imgui_node_editor)

