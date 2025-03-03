cmake_minimum_required(VERSION 3.17)

project(3d-raymarched-engine LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm 0.9 REQUIRED)

add_subdirectory(lib/glad)
add_subdirectory(lib/imgui)
include_directories(lib/cereal/include)
include_directories(lib/imbrowser/include)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/ui.cpp
  src/scene.cpp
  src/viewport.cpp
  src/camera.cpp
  src/shader.cpp
  src/projectdata.cpp
)

file(GLOB_RECURSE SHADER_FILES "src/shaders/*")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders") 
file(GLOB SHADER_FILES "src/shaders/*")
foreach(shader_file ${SHADER_FILES})
    get_filename_component(shader_name ${shader_file} NAME)
    configure_file(
        ${shader_file}
        "${CMAKE_BINARY_DIR}/shaders/${shader_name}"
        COPYONLY
    )
endforeach()

target_link_libraries(${PROJECT_NAME} imgui glfw OpenGL::GL glm::glm glad)

