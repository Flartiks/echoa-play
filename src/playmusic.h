#ifndef PLAYMUSIC_H
#define PLAYMUSIC_H

#include <al.h>
#include <alc.h>
#include <mpg123.h>
#include <vector>
#include <cstdio>
#include <cstdlib> 
#include <string>

bool InitOpenAL();

void CleanupOpenAL();

bool LoadMP3File(const char* filename, ALuint* buffer);
float GetTrackLength(const std::string& filePath);

extern ALCdevice* device;
extern ALCcontext* context;
extern ALuint buffer, source;

#endif // PLAYMUSIC_H