
#include "loadFonts.h"

void LoadRubikFont(ImGuiIO& io) {
    const char* rubikFontPath = RUBIK_FONT_PATH; 
    float fontSize = 16.0f; 

    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 1;
    config.PixelSnapH = true;

    
    static const ImWchar rubik_ranges[] = { 0x0020, 0x00FF, 0x0400, 0x052F, 0 };

    ImFont* rubikFont = io.Fonts->AddFontFromFileTTF(rubikFontPath, fontSize, &config, rubik_ranges);
    if (rubikFont == nullptr) {
        std::cerr << "Failed to load Rubik font from: " << rubikFontPath << std::endl;
    } else {
        std::cout << "Rubik font loaded successfully from: " << rubikFontPath << std::endl;
    }
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
