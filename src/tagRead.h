#ifndef TAGREAD_H
#define TAGREAD_H

#include <iostream>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using std::string;

void ReadMP3Tags(const char* filename, string* title, string* artist, string* album, int* year);
GLuint LoadTextureFromFile(const char* filename);
#endif // TAGREAD_H