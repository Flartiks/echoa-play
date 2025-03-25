#ifndef PLAYMUSIC_H
#define PLAYMUSIC_H

#include <AL/al.h>
#include <AL/alc.h>
#include <mpg123.h>
#include <vector>
#include <cstdio>
#include <cstdlib> 

bool InitOpenAL();

void CleanupOpenAL();

bool LoadMP3File(const char* filename, ALuint* buffer);

extern ALCdevice* device;
extern ALCcontext* context;
extern ALuint buffer, source;

#endif // PLAYMUSIC_H