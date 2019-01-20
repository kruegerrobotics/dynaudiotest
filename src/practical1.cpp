#include <cstdio>
#include <ncurses.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <chrono>

//open AL
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

//SDL sound
#include <SDL/SDL_sound.h>

class SoundEffect
{
  public:
    SoundEffect(std::string filelocation);
    void Play();
    void PlayInfinite();
    ALuint source;

  private:
    bool initialized = false;
    ALuint buffer;
   

    int loadSound(std::string location);
};

void SoundEffect::Play()
{
    if (initialized)
    {
        alSourcePlay(source);
    }
}

void SoundEffect::PlayInfinite()
{
    if (initialized)
    {
        alSourcef(source, AL_LOOPING, 1);
        alSourcePlay(source);
    }
}

int SoundEffect::loadSound(std::string location)
{
    Sound_Sample *sample;
    Uint32 slen;
    ALenum err;
    ALenum format;
    sample = Sound_NewSampleFromFile(location.c_str(), NULL, 65536);
    if (!sample)
    {
        printw("Could not open audio in %s\n", location.c_str());
        return -1;
    }

    /* Get the sound format, and figure out the OpenAL format */
    if (sample->actual.channels == 1)
    {
        if (sample->actual.format == AUDIO_U8)
            format = AL_FORMAT_MONO8;
        else if (sample->actual.format == AUDIO_S16SYS)
            format = AL_FORMAT_MONO16;
        else
        {
            printw("Unsupported sample format: 0x%04x\n", sample->actual.format);
            Sound_FreeSample(sample);
            return -1;
        }
    }
    else if (sample->actual.channels == 2)
    {
        if (sample->actual.format == AUDIO_U8)
            format = AL_FORMAT_STEREO8;
        else if (sample->actual.format == AUDIO_S16SYS)
            format = AL_FORMAT_STEREO16;
        else
        {
            printw("Unsupported sample format: 0x%04x\n", sample->actual.format);
            Sound_FreeSample(sample);
            return -1;
        }
    }
    else
    {
        printw("Unsupported channel count: %d\n", sample->actual.channels);
        Sound_FreeSample(sample);
        return -1;
    }

    /* Decode the whole audio stream to a buffer. */
    slen = Sound_DecodeAll(sample);
    if (!sample->buffer || slen == 0)
    {
        printw("Failed to read audio from %s\n", location);
        Sound_FreeSample(sample);
        return -1;
    }

    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file. */
    buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, sample->buffer, slen, sample->actual.rate);
    Sound_FreeSample(sample);

    /* Check if an error occured, and clean up if so. */
    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        printw("OpenAL Error: %s\n", alGetString(err));
        if (buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return -1;
    }

    return 0;
}

SoundEffect::SoundEffect(std::string filelocation)
{
    int res;

    res = loadSound(filelocation);
    if (res != 0)
    {
        initialized = false;
        return;
    }
    source = 0;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    if (alGetError() != AL_NO_ERROR)
    {
        initialized = false;
        return;
    }
    initialized = true;
}

int kbhit(void)
{
    int ch = getch();

    if (ch != ERR)
    {
        ungetch(ch);
        return 1;
    }
    else
    {
        return 0;
    }
}

int main(void)
{
    float vel = 1.0;
    //init stuff for ncurses
    initscr();

    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);

    //init stuff for open AL
    ALCdevice *device;
    ALCcontext *ctx;
    const ALCchar *name;

    // Open and initialize a device
    device = NULL;
    device = alcOpenDevice(NULL);
    if (!device)
    {
        printw("Could not open a device!\n");
        return 1;
    }

    ctx = alcCreateContext(device, NULL);
    if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if (ctx != NULL)
            alcDestroyContext(ctx);
        alcCloseDevice(device);
        printw("Could not set a context!\n");
        return 1;
    }

    name = NULL;
    if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if (!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);
    printw("Opened \"%s\"\n", name);

    //init the SDL sound lib
    Sound_Init();

    SoundEffect sound1("servo2.wav");
    //SoundEffect sound2("viola.wav");
    sound1.PlayInfinite();
    //sound2.PlayInfinite();

    //the main loop
    float step = 0.02;
    while (1)
    {
        if (kbhit())
        {
            int key = getch();
            if (key == 43)
            {
                vel = vel + step;
                if (vel > 1.0)
                {
                    vel = 1.0;
                }
                //alSourcePause(sound1.source);
                alSourcef(sound1.source, AL_PITCH, vel); 
                //alSourcePlay(sound1.source);
            }
            else if (key == 45)
            {
                vel = vel - step;
                if (vel < 0.0)
                {
                    vel = 0.0;
                }
                alSourcef(sound1.source, AL_PITCH, vel); 
            }
            printw("vel = %f\n", vel);
            refresh();
        }
        else
        {
            refresh();
            usleep(10000);
        }
    }
}