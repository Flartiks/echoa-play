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
using std::string;

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLuint LoadTextureFromFile(const char* filename)
{
    std::cout << "Loading texture from file: " << filename << std::endl;
    GLuint texture = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        0
    );

    if (texture == 0)
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }
    else
    {
        std::cout << "Texture loaded successfully: " << filename << std::endl;
    }

    return texture;
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(650, 600, "hi", NULL, NULL);
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

    if (!InitOpenAL())
    {
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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(550, 500));

        ImGui::Begin("mp3 player", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("To play song, you need to load it first. Enter the path to the file.");
        audiopath[bufferSize - 1] = '\0';
        ImGui::InputText("File Path", audiopath, sizeof(audiopath));
        ImGui::Text("Title: %s", title.c_str());
        ImGui::Text("Artist: %s", artist.c_str());
        ImGui::Text("Album: %s", album.c_str());
        ImGui::Text("Year: %d", year);

        if (ImGui::Button("Load File")) {
            std::cout << "Load File button clicked" << std::endl;
        
            if (strcmp(audiopath, audioFilePath.c_str()) != 0) {
                audioFilePath = audiopath;
        
                if (audioFilePath.empty()) {
                    std::cerr << "Error: File path is empty." << std::endl;
                    ImGui::Text("Error: File path is empty.");
                }
        
                std::cout << "Attempting to load file: " << audioFilePath << std::endl;
    
                CleanupOpenAL();
                if (!InitOpenAL()) {
                    std::cerr << "Failed to initialize OpenAL" << std::endl;
                    ImGui::Text("Error: Failed to initialize OpenAL.");
                }
        
                ReadMP3Tags(audioFilePath.c_str(), &title, &artist, &album, &year);
        
                std::string imagePath = audioFilePath.substr(0, audioFilePath.size() - 4) + ".png";
                extractCoverArt(audioFilePath, imagePath);
        
                if (!LoadMP3File(audioFilePath.c_str(), &buffer)) {
                    std::cerr << "Failed to load audio file: " << audioFilePath << std::endl;
                    ImGui::Text("Error: Failed to load audio file. Please check the file path or format.");
                    isLoaded = false;
                } else isLoaded = true;
        
                alSourceStop(source);
                alSourcei(source, AL_BUFFER, 0);
                alSourcei(source, AL_BUFFER, buffer);
                ALenum error = alGetError();
                if (error != AL_NO_ERROR) {
                    std::cerr << "OpenAL error after setting buffer: " << error << std::endl;
                    ImGui::Text("Error: OpenAL buffer error.");
                }
                
                albumArtTexture = LoadTextureFromFile(imagePath.c_str());
                if (albumArtTexture == 0) {
                    std::cerr << "Failed to load album art texture: " << imagePath << std::endl;
                    ImGui::Text("Error: Failed to load album art texture. Please check the file path or format.");
                } else {
                    std::cout << "Album art texture loaded successfully: " << imagePath << std::endl;
                }
            }
        }
        

        if (ImGui::Button("Play"))
        {
            if(!isLoaded) {
                std::cerr << "Error: No file loaded." << std::endl;
                ImGui::Text("Error: No file loaded.");
            } else {
                std::cout << "Play button clicked!" << std::endl;
                alSourcePlay(source);
                std::cout << "Play file successful!: " << audioFilePath << std::endl;
            }
        }
        if (ImGui::Button("Stop"))
        {
            if(!isLoaded) {
                std::cerr << "Error: No file loaded." << std::endl;
                ImGui::Text("Error: No file loaded.");
            } else {
                printf("Stop button clicked!\n");
                alSourceStop(source);
                printf("Stop file successful!: %s\n", audioFilePath.c_str());
            }
        }
        ImVec2 windowSize = ImGui::GetWindowSize();
        float buttonWidth = ImGui::CalcTextSize("Exit").x + ImGui::GetStyle().FramePadding.x * 2;
        ImGui::SetCursorPos(ImVec2(windowSize.x - buttonWidth - 25, windowSize.y - ImGui::GetFrameHeightWithSpacing()));
        if (ImGui::Button("Exit")) 
        {
            CleanupOpenAL();
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        
            glfwDestroyWindow(window);
            glfwTerminate();
        }

        
        ImVec2 textSize = ImGui::CalcTextSize("Made by Flartiks");
        ImGui::SetCursorPos(ImVec2(windowSize.x - buttonWidth - 200, windowSize.y - ImGui::GetFrameHeightWithSpacing()));
        ImGui::Text("Made by Flartiks");

        if (albumArtTexture != 0) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 260);
            ImGui::Image((ImTextureID)(intptr_t)albumArtTexture, ImVec2(250, 250), ImVec2(0, 0), ImVec2(1, 1));
        } else {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 260);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(250, 250);
            ImGui::Dummy(size);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(255, 165, 0, 255));
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