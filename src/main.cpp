#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <cstdio>
#include <string>
#include "playmusic.h"
#include "tagRead.h"
#include <SOIL/SOIL.h>
#include "albumArt.h"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <vector>
#include <iostream>
#include <shlobj.h> 
#include <shobjidl.h> 
#include <random>
using std::string;

static bool isRepeat = false;
static bool isShuffle = false;
bool isPlayNext = true;

std::vector<std::string> mp3Files; 
std::string selectedDirectory;    
std::string selectedFile;         
std::vector<std::string> remainingTracks;

void LoadRubikFont(ImGuiIO& io) {
    const char* rubikFontPath = RUBIK_FONT_PATH; 
    float fontSize = 16.0f; 

    ImFont* rubikFont = io.Fonts->AddFontFromFileTTF(rubikFontPath, fontSize);
    if (rubikFont == nullptr) {
        std::cerr << "Failed to load Rubik font from: " << rubikFontPath << std::endl;
    } else {
        std::cout << "Rubik font loaded successfully from: " << rubikFontPath << std::endl;
    }
}

void InitializeRemainingTracks() {
    remainingTracks = mp3Files; 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(remainingTracks.begin(), remainingTracks.end(), gen); 
}

static bool isPlaying = false;

std::string OpenFileDialog() {
    char filePath[MAX_PATH] = { 0 };

    OPENFILENAME ofn; 
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; 
    ofn.lpstrFilter = "MP3 Files\0*.mp3\0All Files\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrTitle = "Select an MP3 File";

    if (GetOpenFileName(&ofn)) {
        return std::string(filePath); 
    }

    return ""; 
}

std::string OpenFolderDialogWithIFileDialog() {
    std::string folderPath;
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileDialog* pFileDialog = nullptr;

        
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
        if (SUCCEEDED(hr)) {
            
            DWORD dwOptions;
            pFileDialog->GetOptions(&dwOptions);
            pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

            
            hr = pFileDialog->Show(NULL);
            if (SUCCEEDED(hr)) {
                
                IShellItem* pItem = nullptr;
                hr = pFileDialog->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    
                    PWSTR pszFilePath = nullptr;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        folderPath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileDialog->Release();
        }
        CoUninitialize();
    }

    return folderPath;
}

void LoadFontAwesome(ImGuiIO& io)
{
    const char* fontAwesomePath = FONT_AWESOME_PATH;

    io.Fonts->AddFontDefault();

    ImFontConfig config;
    config.MergeMode = true; 
    config.PixelSnapH = true;
    
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 }; 

    io.Fonts->AddFontFromFileTTF(fontAwesomePath, 16.0f, &config, icons_ranges);
    
    io.Fonts->Build();
}



void ScanDirectoryForMP3(const std::string& directory) {
    mp3Files.clear();
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".mp3") {
            mp3Files.push_back(entry.path().string());
        }
    }
}

void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLuint LoadTextureFromFile(const char* filename) {
    std::cout << "Loading texture from file: " << filename << std::endl;
    GLuint texture = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        0
    );

    if (texture == 0) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    } else {
        std::cout << "Texture loaded successfully: " << filename << std::endl;
    }
    return texture;
}

int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1920, 1079, "echoa-play", NULL, NULL);
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

    ImVec2 defaultWindowPadding = ImVec2(8, 8); 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!InitOpenAL()) {
        fprintf(stderr, "Failed to initialize OpenAL\n");
        return 1;
    }

    std::string audioFilePath;
    GLuint albumArtTexture = 0;
    std::string title, artist, album;
    int year = 0;
    bool isLoaded = false;

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

        if (albumArtTexture != 0) {
            ImGui::Image((ImTextureID)(intptr_t)albumArtTexture, albumArtSize, ImVec2(0, 0), ImVec2(1, 1));
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
        if(isLoaded) {
            ImGui::Spacing();
            ImGui::Text("%s", title.c_str());
            ImGui::Text("%s", artist.c_str());
            ImGui::Text("%s", album.c_str());
            ImGui::Text("%d", year);
        }
        if(!isLoaded) {
            ImGui::Spacing();
            ImGui::Text("No file loaded.");
        }
        ImGui::EndGroup();
        
        ImGui::SameLine(405);
        ImGui::BeginGroup();
        static float volume = 0.5f; 
        if (ImGui::VSliderFloat("##Volume", ImVec2(25, 150), &volume, 0.0f, 1.0f, "")) {
            alSourcef(source, AL_GAIN, volume); 
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        
        static float currentTime = 0.0f;
        static float previousTime = 0.0f; 
        float trackLength = isLoaded ? GetTrackLength(audioFilePath) : 0.0f;

        if (isLoaded) {
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state); 
            if (state == AL_PLAYING) {
                alGetSourcef(source, AL_SEC_OFFSET, &currentTime); 
            }
        }
        int slidePosX = ImGui::GetCursorPosX(); 
        int slidePosY = ImGui::GetCursorPosY(); 
        ImGui::SetCursorPosY(slidePosY - 5); 
        ImGui::SetCursorPosX(slidePosX - 2); 
        
        ImGui::PushItemWidth(425);
        if (ImGui::SliderFloat("##Track Position", &currentTime, 0.0f, trackLength, "Time: %.1f s")) {
            if (isLoaded && std::abs(currentTime - previousTime) > 0.01f) {
                alSourcef(source, AL_SEC_OFFSET, currentTime); 
                previousTime = currentTime; 
            }
        }
        ImGui::PopItemWidth(); 

        ImGui::SetCursorPos(ImVec2(171, 153)); 
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 10.f));
        ImGui::PushFont(io.Fonts->Fonts[0]);
        if (ImGui::Button(u8"\uf048", ImVec2(70, 30))) { 
            if (!mp3Files.empty()) {
                if (isShuffle) {
                    if (remainingTracks.empty()) {
                        InitializeRemainingTracks(); 
                    }
                    selectedFile = remainingTracks.back(); 
                    remainingTracks.pop_back(); 
                } else {
                    auto it = std::find(mp3Files.begin(), mp3Files.end(), selectedFile);
                    if (it != mp3Files.end() && it != mp3Files.begin()) {
                        --it;
                        selectedFile = *it;
                    }
                }
                audioFilePath = selectedFile;
                currentTime = 0.0f;
                CleanupOpenAL();
                if (!InitOpenAL() || !LoadMP3File(audioFilePath.c_str(), &buffer)) {
                    isLoaded = false;
                } else {
                    isLoaded = true;
                    alSourcei(source, AL_BUFFER, buffer);
                    alSourcef(source, AL_GAIN, volume);
                    ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
                    std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
                    extractCoverArt(audioFilePath, imagePath);
                    albumArtTexture = LoadTextureFromFile(imagePath.c_str());
                    alSourcePlay(source); 
                    isPlaying = true;
                }
            }
        }
        ImGui::SameLine();
        
        if (ImGui::Button(isPlaying ? u8"\uf04c" : u8"\uf04b", ImVec2(70, 30))) { 
            if (!isLoaded) {
                std::cerr << "Error: No file loaded." << std::endl;
            } else {
                ALint state;
                alGetSourcei(source, AL_SOURCE_STATE, &state);
                if (state == AL_PLAYING) {
                    alSourcePause(source); 
                    isPlaying = false;
                    std::cout << "Playback paused." << std::endl;
                } else {
                    alSourcePlay(source); 
                    isPlaying = true;
                    std::cout << "Playback started/resumed." << std::endl;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(u8"\uf051", ImVec2(70, 30))) { 
            if (!mp3Files.empty()) {
                if (isShuffle) {
                    if (remainingTracks.empty()) {
                        InitializeRemainingTracks(); 
                    }
                    selectedFile = remainingTracks.back(); 
                    remainingTracks.pop_back(); 
                } else {
                    auto it = std::find(mp3Files.begin(), mp3Files.end(), selectedFile);
                    if (it != mp3Files.end() && it != mp3Files.end() - 1) {
                        ++it;
                        selectedFile = *it;
                    }
                }
                audioFilePath = selectedFile;
                currentTime = 0.0f;
                CleanupOpenAL();
                if (!InitOpenAL() || !LoadMP3File(audioFilePath.c_str(), &buffer)) {
                    isLoaded = false;
                } else {
                    isLoaded = true;
                    alSourcei(source, AL_BUFFER, buffer);
                    alSourcef(source, AL_GAIN, volume);
                    ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
                    std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
                    extractCoverArt(audioFilePath, imagePath);
                    albumArtTexture = LoadTextureFromFile(imagePath.c_str());
                    alSourcePlay(source); 
                    isPlaying = true;
                }
            }
        }

        
        ImGui::SetCursorPos(ImVec2(367, 117)); 
        if (isRepeat) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.7f, 1.0f)); 
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); 
        }
        if (ImGui::Button(u8"\uf074", ImVec2(30, 30))) {
            isRepeat = !isRepeat;
        }
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(ImVec2(367, 82));
        if (isShuffle) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.7f, 1.0f)); 
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); 
        }
        if (ImGui::Button(u8"\uf01e", ImVec2(30, 30))) {
            isShuffle = !isShuffle;
        }
        ImGui::PopStyleColor();
        ImGui::PopFont();
        if (isLoaded) {
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED) {
                if (isRepeat) {
                    alSourcePlay(source); 
                } else {
                    if (isShuffle) {
                        if (remainingTracks.empty()) {
                            InitializeRemainingTracks(); 
                        }
                        selectedFile = remainingTracks.back();
                        remainingTracks.pop_back();
                    } else {
                        auto it = std::find(mp3Files.begin(), mp3Files.end(), selectedFile);
                        if (it != mp3Files.end() && it != mp3Files.end() - 1) {
                            ++it;
                            selectedFile = *it;
                        } else {
                            
                            isPlayNext = false;
                            isPlaying = false;
                        }
                    }
                    audioFilePath = selectedFile;
                    CleanupOpenAL();
                    if (!InitOpenAL() || !LoadMP3File(audioFilePath.c_str(), &buffer)) {
                        isLoaded = false;
                    } else {
                        isLoaded = true;
                        alSourcei(source, AL_BUFFER, buffer);
                        alSourcef(source, AL_GAIN, volume);
                        ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
                        std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
                        extractCoverArt(audioFilePath, imagePath);
                        albumArtTexture = LoadTextureFromFile(imagePath.c_str());
                        if(isPlayNext) {
                            alSourcePlay(source); 
                        } else {
                            alSourceStop(source);
                        }  
                        isPlayNext = true;

                        isPlaying = true;
                    }
                }
            }
        }

        ImGui::PopStyleVar();
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 170); 
        if (ImGui::Button("Choose File", ImVec2(100, 30))) {
            std::string selectedFile = OpenFileDialog();
            if (!selectedFile.empty()) {
                std::cout << "Selected file: " << selectedFile << std::endl;

                audioFilePath = selectedFile;
                CleanupOpenAL();
                if (!InitOpenAL()) {
                    std::cerr << "Failed to initialize OpenAL" << std::endl;
                    isLoaded = false;
                } else if (!LoadMP3File(audioFilePath.c_str(), &buffer)) {
                    std::cerr << "Failed to load audio file: " << audioFilePath << std::endl;
                    isLoaded = false;
                } else {
                    
                    alSourceStop(source);
                    alSourcei(source, AL_BUFFER, 0); 
                    alSourcei(source, AL_BUFFER, buffer);
                    
                    ALenum error = alGetError();
                    if (error != AL_NO_ERROR) {
                        std::cerr << "OpenAL error after setting buffer: " << error << std::endl;
                        isLoaded = false;
                    } else {
                        std::cout << "Buffer successfully attached to source." << std::endl;
                        isLoaded = true;
                    }
                }
                ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
                std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
                extractCoverArt(audioFilePath, imagePath);
                albumArtTexture = LoadTextureFromFile(imagePath.c_str());
                if (albumArtTexture == 0) {
                    std::cerr << "Failed to load album art texture: " << imagePath << std::endl;
                } else {
                    std::cout << "Album art texture loaded successfully: " << imagePath << std::endl;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Choose Folder", ImVec2(100, 30))) {
            std::string selectedFolder = OpenFolderDialogWithIFileDialog(); 
            if (!selectedFolder.empty()) {
                std::cout << "Selected folder: " << selectedFolder << std::endl;
                ScanDirectoryForMP3(selectedFolder); 
            }
        }
        
        if (!mp3Files.empty()) {
            ImGui::Text("MP3 Files:");
            for (size_t i = 0; i < mp3Files.size(); ++i) {
                
                std::string fileName = std::filesystem::path(mp3Files[i]).filename().string();

                if (ImGui::Selectable(fileName.c_str(), selectedFile == mp3Files[i])) {
                    selectedFile = mp3Files[i]; 
                    std::cout << "Selected file: " << selectedFile << std::endl;

                    audioFilePath = selectedFile;
                    CleanupOpenAL();
                    if (!InitOpenAL()) {
                        std::cerr << "Failed to initialize OpenAL" << std::endl;
                        isLoaded = false;
                    } else if (!LoadMP3File(audioFilePath.c_str(), &buffer)) {
                        std::cerr << "Failed to load audio file: " << audioFilePath << std::endl;
                        isLoaded = false;
                    } else {
                        
                        alSourceStop(source);
                        alSourcei(source, AL_BUFFER, 0); 
                        alSourcei(source, AL_BUFFER, buffer);

                        ALenum error = alGetError();
                        if (error != AL_NO_ERROR) {
                            std::cerr << "OpenAL error after setting buffer: " << error << std::endl;
                            isLoaded = false;
                        } else {
                            std::cout << "Buffer successfully attached to source." << std::endl;
                            isLoaded = true;
                        }
                    }
                    
                    ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
                    
                    std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
                    extractCoverArt(audioFilePath, imagePath);
                    albumArtTexture = LoadTextureFromFile(imagePath.c_str());
                    if (albumArtTexture == 0) {
                        std::cerr << "Failed to load album art texture: " << imagePath << std::endl;
                    } else {
                        std::cout << "Album art texture loaded successfully: " << imagePath << std::endl;
                    }
                }
            }
        } else {
            ImGui::Text("No MP3 files found.");
        }
        ImGui::PopFont();
        ImGui::End();
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