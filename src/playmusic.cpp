#include "playmusic.h"
#include <iostream>
#include <cstring>

ALCdevice* device;
ALCcontext* context;
ALuint buffer, source;

bool InitOpenAL()
{
    device = alcOpenDevice(NULL);
    if (!device)
        return false;

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context))
        return false;

    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    return true;
}

void CleanupOpenAL()
{
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

bool LoadMP3File(const char* filename, ALuint* buffer)
{
    if (!filename || strlen(filename) == 0)
    {
        std::cerr << "Error: File path is empty or null." << std::endl;
        return false;
    }

    mpg123_handle *mh;
    unsigned char *audio;
    size_t done;
    int err;
    long rate;
    int channels, encoding;

    mpg123_init();
    mh = mpg123_new(NULL, &err);
    if (!mh)
    {
        std::cerr << "Failed to create mpg123 handle: " << mpg123_plain_strerror(err) << std::endl;
        return false;
    }

    if (mpg123_open(mh, filename) != MPG123_OK)
    {
        std::cerr << "Failed to open MP3 file: " << filename << std::endl;
        mpg123_delete(mh);
        mpg123_exit();
        return false;
    }

    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK)
    {
        std::cerr << "Failed to get MP3 format" << std::endl;
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return false;
    }

    std::vector<unsigned char> pcmData;
    size_t buffer_size = mpg123_outblock(mh);
    audio = (unsigned char*) malloc(buffer_size);

    while (mpg123_read(mh, audio, buffer_size, &done) == MPG123_OK)
    {
        pcmData.insert(pcmData.end(), audio, audio + done);
    }

    free(audio);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    ALenum format;
    if (channels == 1)
        format = AL_FORMAT_MONO16;
    else if (channels == 2)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr << "Unsupported number of channels: " << channels << std::endl;
        return false;
    }

    if (rate <= 0)
    {
        std::cerr << "Invalid sample rate: " << rate << std::endl;
        return false;
    }

    alBufferData(*buffer, format, pcmData.data(), pcmData.size(), rate);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        std::cerr << "OpenAL error after setting buffer data: " << error << std::endl;
        return false;
    }
    std::cout << "Load file successful: " << filename << std::endl;

    return true;
}

float GetTrackLength(const std::string& filePath) {
    mpg123_handle* mh = mpg123_new(NULL, NULL);
    if (!mh) {
        std::cerr << "Failed to initialize mpg123" << std::endl;
        return 0.0f;
    }

    if (mpg123_open(mh, filePath.c_str()) != MPG123_OK) {
        std::cerr << "Failed to open MP3 file: " << filePath << std::endl;
        mpg123_delete(mh);
        return 0.0f;
    }

    // Получаем общее количество фреймов
    off_t totalFrames = mpg123_length(mh);
    if (totalFrames <= 0) {
        std::cerr << "Failed to get total frames" << std::endl;
        mpg123_close(mh);
        mpg123_delete(mh);
        return 0.0f;
    }

    // Получаем частоту дискретизации (в герцах)
    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        std::cerr << "Failed to get MP3 format" << std::endl;
        mpg123_close(mh);
        mpg123_delete(mh);
        return 0.0f;
    }

    // Рассчитываем длину трека в секундах
    double trackLength = static_cast<double>(totalFrames) / static_cast<double>(rate);

    mpg123_close(mh);
    mpg123_delete(mh);

    return static_cast<float>(trackLength); // Возвращаем длину в секундах
}