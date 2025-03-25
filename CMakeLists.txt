cmake_minimum_required(VERSION 3.14)

project(fastPlayMP3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp src/playmusic.cpp src/tagRead.cpp
)

target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

set(OPENAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/OpenAL")
set(OPENAL_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/OpenAL/OpenAL32.lib")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${OPENAL_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
    ${OPENAL_LIBRARY}
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/libmpg123-0.dll" 
)