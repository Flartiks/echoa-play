#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <cstdio>
#include <string>
#include <algorithm>
#include "playmusic.h"
#include "tagRead.h"
#include <SOIL/SOIL.h>
#include "albumArt.h"
#include <vector>
#include <iostream>
#include "files.h"
#include "loadFonts.h"
#include <random>
#include "AppState.hpp"
#include <clocale>
#include <locale>
#include <codecvt>
#ifdef _WIN32
#include <windows.h>
#endif

int AppState::selectedTab = 0;

void LoadTrack(AppState& state, const std::string& path) {
    state.audioFilePath = path;
    CleanupOpenAL();
    if (!InitOpenAL() || !LoadMP3File(path.c_str(), &buffer)) {
        state.isLoaded = false;
        return;
    }

    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, state.volume);
    ReadMP3Tags(path.c_str(), &state.title, &state.artist, &state.album, &state.year);

    std::string imagePath = path.substr(0, path.size() - 4) + ".png";
    extractCoverArt(path, imagePath);
    state.albumArtTexture = LoadTextureFromFile(imagePath.c_str());
    state.isLoaded = true;
    state.currentTime = 0.0f;
}

void TogglePlayPause(AppState& state) {
    ALint stateAL;
    alGetSourcei(source, AL_SOURCE_STATE, &stateAL);
    if (stateAL == AL_PLAYING) {
        alSourcePause(source);
        state.isPlaying = false;
    } else {
        alSourcePlay(source);
        state.isPlaying = true;
    }
}

void InitializeRemainingTracks(AppState& state) {
    state.remainingTracks = state.mp3Files;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(state.remainingTracks.begin(), state.remainingTracks.end(), g);
}

void PlayNextTrack(AppState& state) {
    if (state.mp3Files.empty()) return;

    if (state.isShuffle) {
        if (state.remainingTracks.empty()) InitializeRemainingTracks(state);
        state.selectedFile = state.remainingTracks.back();
        state.remainingTracks.pop_back();
    } else {
        auto it = std::find(state.mp3Files.begin(), state.mp3Files.end(), state.selectedFile);
        if (it != state.mp3Files.end() && it + 1 != state.mp3Files.end()) {
            state.selectedFile = *(it + 1);
        } else return;
    }
    LoadTrack(state, state.selectedFile);
    alSourcePlay(source);
    state.isPlaying = true;
}

void PlayPreviousTrack(AppState& state) {
    if (state.mp3Files.empty()) return;

    if (state.isShuffle) {
        if (state.remainingTracks.empty()) InitializeRemainingTracks(state);
        state.selectedFile = state.remainingTracks.back();
        state.remainingTracks.pop_back();
    } else {
        auto it = std::find(state.mp3Files.begin(), state.mp3Files.end(), state.selectedFile);
        if (it != state.mp3Files.begin()) {
            state.selectedFile = *(it - 1);
        } else return;
    }
    LoadTrack(state, state.selectedFile);
    alSourcePlay(source);
    state.isPlaying = true;
}

void AddMP3FromDirectory(AppState& state, const std::string& directory) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.path().extension() == ".mp3") {
                std::string path = entry.path().u8string(); 
                if (std::find(state.mp3Files.begin(), state.mp3Files.end(), path) == state.mp3Files.end()) {
                    state.mp3Files.push_back(path);
                    
                    std::string title, artist, album;
                    int year;
                    ReadMP3Tags(path.c_str(), &title, &artist, &album, &year);
                    std::string imagePath = path.substr(0, path.size() - 4) + ".png";
                    extractCoverArt(path, imagePath);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

void AddMP3File(AppState& state, const std::string& filePath) {
    try {
        std::filesystem::path path(filePath);
        if (path.extension() == ".mp3") {
            std::string pathStr = path.u8string();
            if (std::find(state.mp3Files.begin(), state.mp3Files.end(), pathStr) == state.mp3Files.end()) {
                state.mp3Files.push_back(pathStr);
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}



void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**) {
    std::setlocale(LC_ALL, "en_US.UTF-8"); 
#ifdef _WIN32
    
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    AppState state;
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "echoa-prem", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    LoadFontAwesome(io);
    LoadRubikFont(io);
    ImGui::StyleColorsDark();
    io.IniFilename = NULL;

    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.5f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.3f, 0.7f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.4f, 0.8f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.3f, 0.7f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

    style.WindowRounding = 10.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(5, 5);
    style.ItemSpacing = ImVec2(8, 8);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.ScrollbarSize = 15.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!InitOpenAL()) {
        fprintf(stderr, "Failed to initialize OpenAL\n");
        return 1;
    }

    
    ImVec2 albumArtSize2 = ImVec2(80, 80);
    static int selectedTab = 0;
    ImVec2 selectableSize(400, 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        
        ImGui::PushFont(io.Fonts->Fonts[1]);
        ImGui::SetNextWindowSize(ImVec2(445, 400));
        ImGui::Begin("echoa-play", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::SetWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
        
        
        ImVec2 albumArtSize = ImVec2(150, 150);
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        if (state.albumArtTexture != 0) {
            ImGui::Image((ImTextureID)(intptr_t)state.albumArtTexture, albumArtSize, ImVec2(0, 0), ImVec2(1, 1));
        } else {
            ImGui::Dummy(albumArtSize);
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            ImU32 borderColor = IM_COL32(255, 255, 255, 255);
            float borderThickness = 2.0f;
            drawList->AddRect(cursorPos, ImVec2(cursorPos.x + albumArtSize.x, cursorPos.y + albumArtSize.y), borderColor, 0.0f, 0, borderThickness);
        }

        
        float tagsOffsetX = cursorPos.x + albumArtSize.x + 20;
        float tagsOffsetY = cursorPos.y;
        ImGui::SetCursorPos(ImVec2(tagsOffsetX - ImGui::GetWindowPos().x, tagsOffsetY - ImGui::GetWindowPos().y));
        ImGui::BeginGroup();
        if(state.isLoaded) {
            ImGui::Spacing();
            ImGui::Text("%s", state.title.c_str());
            ImGui::Text("%s", state.artist.c_str());
            ImGui::Text("%s", state.album.c_str());
            ImGui::Text("%d", state.year);
        }
        if(!state.isLoaded) {
            ImGui::Spacing();
            ImGui::Text("No file loaded.");
        }
        ImGui::EndGroup();
        
        
        ImGui::SameLine(405);
        ImGui::BeginGroup();
        if (ImGui::VSliderFloat("##Volume", ImVec2(25, 150), &state.volume, 0.0f, 1.0f, "")) {
            alSourcef(source, AL_GAIN, state.volume);
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        
        
        float trackLength = state.isLoaded ? GetTrackLength(state.audioFilePath) : 0.0f;
        if (state.isLoaded) {
            ALint state_al;
            alGetSourcei(source, AL_SOURCE_STATE, &state_al);
            if (state_al == AL_PLAYING) {
                alGetSourcef(source, AL_SEC_OFFSET, &state.currentTime);
            }
        }
        int slidePosX = ImGui::GetCursorPosX();
        int slidePosY = ImGui::GetCursorPosY();
        ImGui::SetCursorPosY(slidePosY - 5);
        ImGui::SetCursorPosX(slidePosX - 2);
        
        ImGui::PushItemWidth(425);
        if (ImGui::SliderFloat("##Track Position", &state.currentTime, 0.0f, trackLength, "Time: %.1f s")) {
            if (state.isLoaded && std::abs(state.currentTime - state.previousTime) > 0.01f) {
                alSourcef(source, AL_SEC_OFFSET, state.currentTime);
                state.previousTime = state.currentTime;
            }
        }
        ImGui::PopItemWidth();

        
        ImGui::SetCursorPos(ImVec2(171, 153));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 10.f));
        ImGui::PushFont(io.Fonts->Fonts[0]);
        if (ImGui::Button(u8"\uf048", ImVec2(70, 30))) { 
            PlayPreviousTrack(state);
        }
        ImGui::SameLine();
        if (ImGui::Button(state.isPlaying ? u8"\uf04c" : u8"\uf04b", ImVec2(70, 30))) { 
            TogglePlayPause(state);
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"\uf051", ImVec2(70, 30))) { 
            PlayNextTrack(state);
        }

        
        ImGui::SetCursorPos(ImVec2(367, 117));
        if (state.isRepeat) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        }
        if (ImGui::Button(u8"\uf01e", ImVec2(30, 30))) {
            state.isRepeat = !state.isRepeat;
        }
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(ImVec2(367, 82));
        if (state.isShuffle) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        }
        if (ImGui::Button(u8"\uf074", ImVec2(30, 30))) {
            state.isShuffle = !state.isShuffle;
        }
        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::PopStyleVar();

        
        if (state.isLoaded) {
            ALint state_al;
            alGetSourcei(source, AL_SOURCE_STATE, &state_al);
            if (state_al == AL_STOPPED) {
                if (state.isRepeat) {
                    alSourcePlay(source);
                } else {
                    PlayNextTrack(state);
                }
            }
        }

        
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 170);
        if (ImGui::Button("Choose File", ImVec2(100, 30))) {
            std::string selectedFile = OpenFileDialog();
            if (!selectedFile.empty()) {
                state.selectedFile = selectedFile;
                AddMP3File(state, selectedFile);
                LoadTrack(state, selectedFile);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Choose Folder", ImVec2(100, 30))) {
            std::string selectedFolder = OpenFolderDialogWithIFileDialog();
            if (!selectedFolder.empty()) {
                AddMP3FromDirectory(state, selectedFolder);
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Update echoa-prem chunk", ImVec2(200, 30))) {
            std::string imagePath = state.audioFilePath.substr(0, state.audioFilePath.size() - 4) + ".png";
            state.albumArtTexture2 = LoadTextureFromFile(imagePath.c_str());
            state.needToRefresh = true;
        }

        
        if (!state.mp3Files.empty()) {
            ImGui::Text("MP3 Files:");
            for (size_t i = 0; i < state.mp3Files.size(); ++i) {
                std::string fileName = std::filesystem::path(state.mp3Files[i]).filename().string();

                ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(100, 150, 255, 200));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(120, 180, 255, 255));
                ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, IM_COL32(80, 130, 230, 255));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

                if (ImGui::Selectable(fileName.c_str(), state.selectedFile == state.mp3Files[i], 0, selectableSize)) {
                    state.selectedFile = state.mp3Files[i];
                    LoadTrack(state, state.selectedFile);
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(3);

                
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImVec2 lineStart = ImVec2(min.x, max.y);
                ImVec2 lineEnd = ImVec2(min.x + selectableSize.x, max.y);
                ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, IM_COL32(100, 100, 100, 80), 1.0f);
            }
        } else {
            ImGui::Text("No MP3 files found.");
        }
        ImGui::PopFont();
        ImGui::End();

        
        ImGui::SetNextWindowSize(ImVec2(1280, 100));
        ImGui::SetNextWindowPos(ImVec2(0, 620), ImGuiCond_FirstUseEver);
        ImGui::PushFont(io.Fonts->Fonts[1]);
        ImGui::Begin("echoa-prem", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar);
        ImGui::SetCursorPos(ImVec2(10,10));

        if (state.needToRefresh) {
            if (state.albumArtTexture2 != 0) {
                ImDrawList* dl = ImGui::GetForegroundDrawList();
                ImVec2 p_min = ImGui::GetCursorScreenPos();
                ImVec2 p_max = ImVec2(p_min.x + 80, p_min.y + 80);
                dl->AddImageRounded((ImTextureID)(intptr_t)state.albumArtTexture2, p_min, p_max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)), 10);
            }
            else {
                ImGui::Dummy(albumArtSize2);
                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                ImU32 borderColor = IM_COL32(255, 255, 255, 255);
                float borderThickness = 2.0f;
                drawList->AddRect(ImVec2(10,630), ImVec2(10.f + albumArtSize2.x, 630.f + albumArtSize2.y), borderColor, 0.0f, 0, borderThickness);
            }

            ImGui::SameLine();

            ImGuiStyle& style = ImGui::GetStyle();
            float originalItemSpacingY = style.ItemSpacing.y;

            style.ItemSpacing.y = 0.5f;

            ImGui::BeginGroup();
            ImGui::Dummy(ImVec2(0.0f, 48.f));
            ImGui::SetCursorPosX(98.f);
            ImGui::Text("%s", state.title.c_str());
            ImGui::SetCursorPosX(98.f);
            float slideposx2 = ImGui::GetCursorPosX() + 270.f;
            float slideposy2 = ImGui::GetCursorPosY() - 10.f;
            ImGui::Text("%s", state.artist.c_str());
            ImGui::EndGroup();

            style.ItemSpacing.y = originalItemSpacingY;
            ImGui::PushItemWidth(600);
            ImGui::SetCursorPos(ImVec2(slideposx2, slideposy2));
            float trackLength = state.isLoaded ? GetTrackLength(state.audioFilePath) : 0.0f;
            if (ImGui::SliderFloat("##Track Position", &state.currentTime, 0.0f, trackLength, "Time: %.1f s")) {
                if (state.isLoaded && std::abs(state.currentTime - state.previousTime) > 0.01f) {
                    alSourcef(source, AL_SEC_OFFSET, state.currentTime);
                    state.previousTime = state.currentTime;
                }
            }
            ImGui::PopItemWidth();

            ImGui::SetCursorPos(ImVec2(550, 25));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 10.f));
            ImGui::PushFont(io.Fonts->Fonts[0]);
            if (ImGui::Button(u8"\uf048", ImVec2(70, 30))) {
                PlayPreviousTrack(state);
            }
            ImGui::SameLine();
            if (ImGui::Button(state.isPlaying ? u8"\uf04c" : u8"\uf04b", ImVec2(70, 30))) {
                TogglePlayPause(state);
            }
            ImGui::SameLine();
            if (ImGui::Button(u8"\uf051", ImVec2(70, 30))) {
                PlayNextTrack(state);
            }

            ImGui::SetCursorPos(ImVec2(510, 25));
            if (state.isRepeat) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            }
            if (ImGui::Button(u8"\uf01e", ImVec2(30, 30))) {
                state.isRepeat = !state.isRepeat;
            }
            ImGui::PopStyleColor();

            ImGui::SetCursorPos(ImVec2(785, 25));
            if (state.isShuffle) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            }
            if (ImGui::Button(u8"\uf074", ImVec2(30, 30))) {
                state.isShuffle = !state.isShuffle;
            }
            ImGui::PopStyleColor();
            ImGui::PopFont();
            ImGui::PopStyleVar();
        }
        
        ImGui::PopFont();
        ImGui::End();

        
        ImGui::SetNextWindowSize(ImVec2(600, 480));
        ImGui::SetNextWindowPos(ImVec2(368, 140), ImGuiCond_FirstUseEver);
        ImGui::PushFont(io.Fonts->Fonts[1]);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 10));
        ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.15f, 0.25f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.2f, 0.3f, 0.5f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.3f, 0.5f, 0.8f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(0.15f, 0.2f, 0.3f, 0.3f));

        ImGui::Begin("files", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("MainTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Locale files", nullptr, state.selectedTab == 0 ? ImGuiTabItemFlags_SetSelected : 0)) {
                float starttablocaleX = ImGui::GetCursorPosX();
                float starttablocaleY = ImGui::GetCursorPosY();
                ImVec2 starttablocale = ImGui::GetCursorScreenPos();
                state.selectedTab = 0;
                ImGui::PushFont(io.Fonts->Fonts[0]);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, -5.f));
                ImGui::SetCursorPosX(555.f);
                if (ImGui::Button(u8"\uf15b", ImVec2(30, 35))) {
                    std::string selectedFile = OpenFileDialog();
                    if (!selectedFile.empty()) {
                        AddMP3File(state, selectedFile);
                        state.selectedFile = selectedFile;
                        LoadTrack(state, selectedFile);
                        alSourcePlay(source);
                        state.isPlaying = true;
                    }
                }
                ImGui::SetCursorPosX(555.f);
                if (ImGui::Button(u8"\uf07b", ImVec2(30, 35))) {
                    std::string selectedFolder = OpenFolderDialogWithIFileDialog();
                    if (!selectedFolder.empty()) {
                        AddMP3FromDirectory(state, selectedFolder);
                    }
                }
                ImGui::PopFont();
                ImGui::PopStyleVar();
                ImGui::SetCursorPosY(starttablocaleY);

                if (!state.mp3Files.empty()) {
                    for (size_t i = 0; i < state.mp3Files.size(); ++i) {
                        std::string fileName = std::filesystem::path(state.mp3Files[i]).filename().string();

                        ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(100, 150, 255, 200));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(120, 180, 255, 255));
                        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, IM_COL32(80, 130, 230, 255));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

                        if (ImGui::Selectable(fileName.c_str(), state.selectedFile == state.mp3Files[i], 0, selectableSize)) {
                            state.selectedFile = state.mp3Files[i];
                            LoadTrack(state, state.selectedFile);
                            alSourcePlay(source);
                            state.isPlaying = true;
                        }
                        ImGui::PopStyleColor(3);
                        ImGui::PopStyleVar();

                        ImVec2 min = ImGui::GetItemRectMin();
                        ImVec2 max = ImGui::GetItemRectMax();
                        ImVec2 lineStart = ImVec2(min.x, max.y);
                        ImVec2 lineEnd = ImVec2(min.x + selectableSize.x, max.y);
                        ImGui::GetWindowDrawList()->AddLine(lineStart, lineEnd, IM_COL32(100, 100, 100, 80), 1.0f);
                    }
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Playlist", nullptr, state.selectedTab == 1 ? ImGuiTabItemFlags_SetSelected : 0)) {
                ImGui::Text("Будущая вкладка для песен из интернета");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(4);
        ImGui::PopFont();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    CleanupOpenAL();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}