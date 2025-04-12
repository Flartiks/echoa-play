#include "tagRead.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using std::string;

void ReadMP3Tags(const char* filename, string* title, string* artist, string* album, int* year) {
    TagLib::FileRef f(filename);
    if (!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        *title = tag->title().to8Bit(true);
        *artist = tag->artist().to8Bit(true);
        *album = tag->album().to8Bit(true);
        *year = tag->year();
    } else {
        std::cerr << "Error reading file or tag: " << filename << std::endl;
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