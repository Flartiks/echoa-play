#ifndef TAGREAD_H
#define TAGREAD_H

#include <iostream>
using std::string;

void ReadMP3Tags(const char* filename, string* title, string* artist, string* album, int* year);

#endif // TAGREAD_H