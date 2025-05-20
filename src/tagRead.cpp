#include "tagRead.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#ifdef _WIN32
#include <codecvt>
#include <locale>
#endif

using std::string;

void ReadMP3Tags(const char* filename, string* title, string* artist, string* album, int* year) {
    std::cerr << "Attempting to read tags for: " << filename << std::endl;

#ifdef _WIN32
    
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wfilename = converter.from_bytes(filename);
    TagLib::FileRef f(wfilename.c_str());
#else
    TagLib::FileRef f(filename);
#endif

    if (!f.isNull() && f.tag()) {
        TagLib::Tag* tag = f.tag();
        *title = tag->title().to8Bit(true);
        *artist = tag->artist().to8Bit(true);
        *album = tag->album().to8Bit(true);
        *year = tag->year();
        std::cerr << "Tags read successfully: Title=" << *title << ", Artist=" << *artist 
                  << ", Album=" << *album << ", Year=" << *year << std::endl;
    } else {
        std::cerr << "Error reading file or tag: " << filename << std::endl;
        if (f.isNull()) {
            std::cerr << "TagLib::FileRef is null (file could not be opened)" << std::endl;
        } else if (!f.tag()) {
            std::cerr << "No valid tags found in file" << std::endl;
        }
        
        *title = "Unknown Title";
        *artist = "Unknown Artist";
        *album = "Unknown Album";
        *year = 0;
    }
}

GLuint LoadTextureFromFile(const char* filename) {
    GLuint texture = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        0
    );

    if (texture == 0) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }
    return texture;
}