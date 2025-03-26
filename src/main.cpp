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
#include "ImGuiFileDialog.h"
#include <filesystem>
#include <vector>
using std::string;

std::vector<std::string> mp3Files; // Список MP3-файлов
std::string selectedDirectory;    // Выбранная директория
std::string selectedFile;         // Выбранный MP3-файл

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

    GLFWwindow* window = glfwCreateWindow(1920, 1000, "hi", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); 
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); 
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); 
    style.TabRounding = 5.f;
    style.FrameRounding = 5.f;
    style.GrabRounding = 5.f;
    style.WindowRounding = 5.f;
    style.PopupRounding = 5.f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    if (!InitOpenAL()) {
        fprintf(stderr, "Failed to initialize OpenAL\n");
        return 1;
    }

    std::string audioFilePath = "ff";
    const int bufferSize = 256;
    char audiopath[bufferSize] = {0};
    alSourcei(source, AL_BUFFER, buffer);

    std::string title, artist, album;
    int year = 0;

    GLuint albumArtTexture = 0;

    bool isLoaded = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Окно выбора директории
        ImGui::Begin("Directory Choose", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowSize(ImVec2(400, 300));
        ImGui::SetWindowPos(ImVec2(650, 50));
        if (ImGui::Button("Choose Directory")) {
            ImGuiFileDialog::Instance()->OpenDialog(
                "ChooseDir",                      // Идентификатор диалога
                "Choose Directory",               // Заголовок окна
                nullptr,                          // Фильтры (nullptr для директорий)    
                IGFD::FileDialogConfig()          // Конфигурация диалога
            );
        }
        
        // Обработка выбора директории
        if (ImGuiFileDialog::Instance()->Display("ChooseDir")) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                selectedDirectory = ImGuiFileDialog::Instance()->GetCurrentPath();
                ScanDirectoryForMP3(selectedDirectory); // Сканируем директорию на наличие MP3-файлов
            }
            ImGuiFileDialog::Instance()->Close();
        }
        ImGui::Text("Selected Directory: %s", selectedDirectory.c_str());

        // Отображение списка MP3-файлов
        ImGui::Text("MP3 Files:");
        if (!mp3Files.empty()) {
            for (size_t i = 0; i < mp3Files.size(); ++i) {
                if (ImGui::Selectable(mp3Files[i].c_str(), selectedFile == mp3Files[i])) {
                    selectedFile = mp3Files[i]; // Выбираем MP3-файл
                }
            }
        } else {
            ImGui::Text("No MP3 files found.");
        }

        if (!selectedFile.empty() && ImGui::Button("Load Selected File")) {
            // Логика загрузки выбранного MP3-файла
            audioFilePath = selectedFile;
            std::cout << "Selected file to load: " << audioFilePath << std::endl;
        
            // Очищаем предыдущие данные
            CleanupOpenAL();
            if (!InitOpenAL()) {
                std::cerr << "Failed to initialize OpenAL" << std::endl;
                ImGui::Text("Error: Failed to initialize OpenAL.");
                isLoaded = false;
                return 1;
            }
        
            // Читаем теги MP3-файла
            ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
        
            // Извлекаем обложку альбома
            std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
            extractCoverArt(audioFilePath, imagePath);
        
            // Загружаем MP3-файл в OpenAL
            if (!LoadMP3File(audioFilePath.c_str(), &buffer)) {
                std::cerr << "Failed to load audio file: " << audioFilePath << std::endl;
                ImGui::Text("Error: Failed to load audio file. Please check the file path or format.");
                isLoaded = false;
            } else {
                isLoaded = true;
            }
        
            // Привязываем буфер к источнику
            alSourceStop(source);
            alSourcei(source, AL_BUFFER, 0);
            alSourcei(source, AL_BUFFER, buffer);
            ALenum error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cerr << "OpenAL error after setting buffer: " << error << std::endl;
                ImGui::Text("Error: OpenAL buffer error.");
            }
        
            // Загружаем текстуру обложки альбома
            albumArtTexture = LoadTextureFromFile(imagePath.c_str());
            if (albumArtTexture == 0) {
                std::cerr << "Failed to load album art texture: " << imagePath << std::endl;
                ImGui::Text("Error: Failed to load album art texture. Please check the file path or format.");
            } else {
                std::cout << "Album art texture loaded successfully: " << imagePath << std::endl;
            }
        }

        ImGui::End();

        // Основное окно "fastPlayMP3"
        ImGui::SetNextWindowSize(ImVec2(550, 350));
        ImGui::Begin("fastPlayMP3", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowPos(ImVec2(50, 50));

        // Отображение обложки альбома
        ImVec2 albumArtSize = ImVec2(150, 150); // Размер обложки
        ImVec2 cursorPos = ImGui::GetCursorScreenPos(); // Позиция текущего элемента

        if (albumArtTexture != 0) {
            ImGui::Image((ImTextureID)(intptr_t)albumArtTexture, albumArtSize, ImVec2(0, 0), ImVec2(1, 1));
            ImGui::Dummy(ImVec2(0, 20)); // Резервируем место под текст "No Cover Art"
        } else {
            ImGui::Dummy(albumArtSize); // Пустое место для обложки
            ImGui::Text("No Cover Art");

            // Рисуем рамку вокруг пустого места
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            ImU32 borderColor = IM_COL32(255, 255, 255, 255); // Белый цвет рамки
            float borderThickness = 2.0f; // Толщина рамки
            drawList->AddRect(cursorPos, ImVec2(cursorPos.x + albumArtSize.x, cursorPos.y + albumArtSize.y), borderColor, 0.0f, 0, borderThickness);
        }

        // Информация о треке
        ImGui::Text("Title: %s", title.c_str());
        ImGui::Text("Artist: %s", artist.c_str());
        ImGui::Text("Album: %s", album.c_str());
        ImGui::Text("Year: %d", year);

        ImGui::Spacing();
        if (ImGui::Button("Play", ImVec2(70, 30))) {
            if (!isLoaded) {
                std::cerr << "Error: No file loaded." << std::endl;
                ImGui::Text("Error: No file loaded.");
            } else {
                std::cout << "Play button clicked!" << std::endl;
                alSourcePlay(source);
                std::cout << "Play file successful!: " << audioFilePath << std::endl;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop", ImVec2(70, 30))) {
            if (!isLoaded) {
                std::cerr << "Error: No file loaded." << std::endl;
                ImGui::Text("Error: No file loaded.");
            } else {
                printf("Stop button clicked!\n");
                alSourceStop(source);
                printf("Stop file successful!: %s\n", audioFilePath.c_str());
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause", ImVec2(70, 30))) {
            if (!isLoaded) {
                std::cerr << "Error: No file loaded." << std::endl;
                ImGui::Text("Error: No file loaded.");
            } else {
                ALint state;
                alGetSourcei(source, AL_SOURCE_STATE, &state);
                if (state == AL_PLAYING) {
                    alSourcePause(source);
                    std::cout << "Playback paused." << std::endl;
                } else if (state == AL_PAUSED) {
                    alSourcePlay(source);
                    std::cout << "Playback resumed." << std::endl;
                }
            }
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Previous", ImVec2(100, 30))) {
            if (!mp3Files.empty()) {
                auto it = std::find(mp3Files.begin(), mp3Files.end(), selectedFile);
                if (it != mp3Files.end() && it != mp3Files.begin()) {
                    --it; // Переходим к предыдущему треку
                    selectedFile = *it;
                    std::cout << "Switched to previous track: " << selectedFile << std::endl;
        
                    // Загружаем предыдущий трек
                    audioFilePath = selectedFile;
                    CleanupOpenAL();
                    if (!InitOpenAL()) {
                        std::cerr << "Failed to initialize OpenAL" << std::endl;
                        isLoaded = false;
                    } else if (!LoadMP3File(audioFilePath.c_str(), &buffer)) {
                        std::cerr << "Failed to load audio file: " << audioFilePath << std::endl;
                        isLoaded = false;
                    } else {
                        isLoaded = true;
                        alSourcei(source, AL_BUFFER, buffer);
                        alSourcePlay(source);
        
                        // Обновляем теги MP3-файла
                        ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
        
                        // Обновляем обложку альбома
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
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Next", ImVec2(100, 30))) {
            if (!mp3Files.empty()) {
                auto it = std::find(mp3Files.begin(), mp3Files.end(), selectedFile);
                if (it != mp3Files.end() && it != mp3Files.end() - 1) {
                    ++it; // Переходим к следующему треку
                    selectedFile = *it;
                    std::cout << "Switched to next track: " << selectedFile << std::endl;
        
                    // Загружаем следующий трек
                    audioFilePath = selectedFile;
                    CleanupOpenAL();
                    if (!InitOpenAL()) {
                        std::cerr << "Failed to initialize OpenAL" << std::endl;
                        isLoaded = false;
                    } else if (!LoadMP3File(audioFilePath.c_str(), &buffer)) {
                        std::cerr << "Failed to load audio file: " << audioFilePath << std::endl;
                        isLoaded = false;
                    } else {
                        isLoaded = true;
                        alSourcei(source, AL_BUFFER, buffer);
                        alSourcePlay(source);
        
                        // Обновляем теги MP3-файла
                        ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
        
                        // Обновляем обложку альбома
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
            }
        }

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