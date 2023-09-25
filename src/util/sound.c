#include <stdio.h>
#include <stdlib.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include "sound.h"

int unload_sound_file(sound_element_t sound);
sound_element_t load_sound_file(char *file);

int free_sound(sounds_t sound) 
{
    for (int i = 0; i < SOUND_MAX; i++) 
        unload_sound_file(sound.sounds[i]);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(sound.context);
    alcCloseDevice(sound.device);
    return 0;
}

int unload_sound_file(sound_element_t sound)
{
    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);
    return 0;
}

void sound_pause(sounds_t sound, enum sounds_enum i) 
{
    return alSourcePause(sound.sounds[i].source);
}

char isSoundPlaying(sounds_t sound, enum sounds_enum i)
{
    int state; 
    alGetSourcei(sound.sounds[i].source, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING) ? 1: 0;
}

void sound_play(sounds_t sound, enum sounds_enum i)
{
    return alSourcePlay(sound.sounds[i].source);
}

sound_element_t load_sound_file(char *file)
{
    sound_element_t sound; 
    alGenSources((ALuint)1, &sound.source);

    alSourcef(sound.source, AL_PITCH, 1);
    alSourcef(sound.source, AL_GAIN, 1);
    alSource3f(sound.source, AL_POSITION, 0, 0, 0);
    alSource3f(sound.source, AL_VELOCITY, 0, 0, 0);
    alSourcei(sound.source, AL_VELOCITY, AL_FALSE);

    alGenBuffers((ALuint)1, &sound.buffer);

    ALsizei size, freq;
    ALenum format;
    ALvoid *data;
    ALboolean loop = AL_FALSE;

    alutLoadWAVFile(file, &format, &data, &size, &freq, &loop);
    alBufferData(sound.buffer, format, data, size, freq);

    alSourcei(sound.source, AL_BUFFER, sound.buffer);
    return sound;
}

sounds_t sound_init()
{
    sounds_t sound;
    sound.device = alcOpenDevice(NULL);
    if (!sound.device) 
    {
        perror("Error: opening device\n");
        exit(1);
    }

    sound.context = alcCreateContext(sound.device, NULL);
    if (!alcMakeContextCurrent(sound.context)) 
    {
        perror("Error: setting context current\n");
        exit(1);
    }

    sound.sounds[SOUND_LAUNCH]      = load_sound_file("resources/launch.wav");
    sound.sounds[SOUND_SHOOT]       = load_sound_file("resources/shoot.wav");
    sound.sounds[SOUND_BREAK]       = load_sound_file("resources/break.wav");
    sound.sounds[SOUND_GAME_OVER]   = load_sound_file("resources/game_over.wav");
    sound.sounds[SOUND_SELECT]      = load_sound_file("resources/select.wav");
    return sound;
}

