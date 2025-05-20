cmake_minimum_required(VERSION 3.14)

project(echoa-play)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FONT_AWESOME_PATH "${CMAKE_CURRENT_SOURCE_DIR}/fonts/fontawesome-free-6.7.2-desktop/otfs/Font Awesome 6 Free-Solid-900.otf")
add_definitions(-DFONT_AWESOME_PATH="${FONT_AWESOME_PATH}")
set(RUBIK_FONT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/fonts/rubik/Rubik-Medium.ttf")
add_definitions(-DRUBIK_FONT_PATH="${RUBIK_FONT_PATH}")

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

FetchContent_Declare(
  soil
  GIT_REPOSITORY https://github.com/littlstar/soil.git
  GIT_TAG master
)
FetchContent_MakeAvailable(soil)

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/playmusic.cpp src/playmusic.h
    src/tagRead.cpp src/tagRead.h
    src/albumArt.cpp src/albumArt.h
    src/loadFonts.cpp src/loadFonts.h
    src/files.cpp src/files.h
    resources/resources.rc
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

set(TAGLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/taglib")
set(TAGLIB_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/taglib")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${OPENAL_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${soil_SOURCE_DIR}/src
    ${soil_SOURCE_DIR}/include
    ${TAGLIB_INCLUDE_DIR}
)

add_library(SOIL STATIC
    ${soil_SOURCE_DIR}/src/SOIL.c
    ${soil_SOURCE_DIR}/src/image_DXT.c
    ${soil_SOURCE_DIR}/src/image_helper.c
    ${soil_SOURCE_DIR}/src/stb_image_aug.c
)

target_include_directories(SOIL PUBLIC ${soil_SOURCE_DIR}/src ${soil_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
    ${OPENAL_LIBRARY}
    "${CMAKE_CURRENT_SOURCE_DIR}/libmpg123-0.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/libtag.dll"
    "${CMAKE_CURRENT_SOURCE_DIR}/openAL32.dll"
    SOIL
    OpenGL::GL
)

add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)