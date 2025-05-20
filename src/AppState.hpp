#pragma once
#include <string>
#include <vector>
#include <GLFW/glfw3.h> 
#include <al.h>      
#include <unordered_map>

struct AppState {
    std::vector<std::string> mp3Files;
    std::vector<std::string> remainingTracks;

    std::string selectedFile;
    std::string audioFilePath;
    
    static int selectedTab;
    
    std::string title, artist, album;
    int year = 0;
    
    bool isRepeat = false;
    bool isShuffle = false;
    bool isPlayNext = true;
    bool isPlaying = false;
    bool isLoaded = false;
    bool needToRefresh = false;
    
    float currentTime = 0.0f;
    float previousTime = 0.0f;
    float volume = 0.5f;

    GLuint albumArtTexture = 0;
    GLuint albumArtTexture2 = 0;

    ALuint buffer = 0;

    std::unordered_map<std::string, GLuint> albumArtTextures; 
    std::unordered_map<std::string, std::vector<unsigned char>> albumArtData;
};
