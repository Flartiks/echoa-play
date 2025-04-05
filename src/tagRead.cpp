#include "tagRead.h"
#include <mpg123.h>
#include <iostream>
#include <fstream>
#include <cstring>

using std::string;

void ReadMP3Tags(const char* filename, string* title, string* artist, string* album, int* year) {
    mpg123_init();
    mpg123_handle *mh = mpg123_new(NULL, NULL);
    if (mpg123_open(mh, filename) != MPG123_OK) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        mpg123_delete(mh);
        mpg123_exit();
        return;
    }

    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streampos fileSize = file.tellg();
        if (fileSize > 128) {
            file.seekg(fileSize - 128);
            char tag[128];
            file.read(tag, 128);

            if (strncmp(tag, "TAG", 3) == 0) { 
                *title = string(tag + 3, 30);
                *artist = string(tag + 33, 30);
                *album = string(tag + 63, 30);
                *year = std::atoi(string(tag + 93, 4).c_str());
            } else {
                std::cerr << "No ID3v1 tag found in " << filename << std::endl;
            }
        }
        file.close();
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}