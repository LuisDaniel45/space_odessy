#include <stdio.h>
#include <stdlib.h>

#ifdef linux
#include <AL/al.h>
#include <AL/alc.h>
#include <assert.h>
#endif

#include "sound.h"

#define ERR(msg, ...) {fprintf(stderr, msg); return -1;}

static int read_file(char *file_name, unsigned char **file_content);
char check_seqence(char *sequence, int sequence_size, char *buffer);
char search_sequence(char *sequence, int sequence_size, char *buffer, int buffer_size);

int read_riff_chunk(wav_file_info_t *info);
int read_fmt_subchunk(wav_file_info_t *info);
int read_data_subchunk(wav_file_info_t *info);
int load_wav_file(char *file, wav_file_info_t *info);

int unload_sound_file(sound_element_t sound);
sound_element_t load_sound_file(char *file);

#ifdef linux
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
    free(sound.info.file_data);
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

    assert(load_wav_file(file, &sound.info) == 0);
    ALenum format = AL_FORMAT_MONO8 + ((sound.info.num_channels - 1) * 2) + 
                                      ((sound.info.bits_per_sample/8) - 1); 

    alBufferData(sound.buffer, format, 
                 sound.info.data, 
                 sound.info.data_size, 
                 sound.info.sample_rate);
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
#else

int free_sound(sounds_t sound){} 
int unload_sound_file(sound_element_t sound){}
void sound_pause(sounds_t sound, enum sounds_enum i) {}

char isSoundPlaying(sounds_t sound, enum sounds_enum i) {}
void sound_play(sounds_t sound, enum sounds_enum i) {}
sounds_t sound_init() {}
#endif 


int load_wav_file(char *file, wav_file_info_t *info)
{
    info->file_size = read_file(file, &info->file_data); 
    if (info->file_size == -1) 
        return 1;

    if (read_riff_chunk(info))
    {
        free(info->file_data);
        return 1;
    }

    if (read_fmt_subchunk(info))
    {
        free(info->file_data);
        return 1;
    }
    if (read_data_subchunk(info))
    {
        free(info->file_data);
        return 1;
    }

    return 0;
}

int read_riff_chunk(wav_file_info_t *info)
{
    if(check_seqence("RIFF", 4, info->file_data))
        ERR("Error: Sequence Riff Fail")
    info->data = info->file_data + 4;

    if (*(int*)info->data != info->file_size - 8)
        ERR("Error: Size does not match with chunk size\n");
    info->data += 4;

    if(check_seqence("WAVE", 4, info->data))
        ERR("Error: Sequence Riff fail\n");
    info->data +=  4;

    return 0;
}

int read_fmt_subchunk(wav_file_info_t *info)
{
    if(check_seqence("fmt ", 4, info->data))
        ERR("Error: Sequence fmt fail");
    info->data += 4;

    short subchunk_size = *(int*)info->data;
    info->data += 4;

    if (*(short*)info->data != 1)
        ERR("Error: Audio format not supported, only pcm");
    info->data += 2;

    info->num_channels = *(short*)info->data;
    info->data += 2;

    info->sample_rate = *(int*)info->data;
    info->data += 4;

    info->byte_rate =  *(int*)info->data;
    info->data += 4;

    info->block_align =  *(short*)info->data;
    info->data += 2;

    info->bits_per_sample =  *(short*)info->data;
    info->data += 2;

    int expected_byte_rate = info->sample_rate * info->num_channels * info->bits_per_sample / 8;
    if (info->byte_rate != expected_byte_rate) 
        ERR("Error: Byte rate does not match with expected value\n");

    int expected_block_align = info->num_channels * info->bits_per_sample / 8;
    if (info->block_align != expected_block_align) 
        ERR("Error: Block align and expected does not match\n");

    return 0;
}

int read_data_subchunk(wav_file_info_t *info)
{

    int ret = search_sequence("data", 4, info->data, info->file_size);
    if (!ret) 
        ERR("Error: data chunk not found\n");
    info->data += ret + 1;

    info->data_size = *(int*)info->data;
    info->data += 4;

    return 0;
}


char search_sequence(char *sequence, int sequence_size, char *buffer, int buffer_size)
{
    for (int i = 0, counter = 0; i < buffer_size; i++) {
        if (sequence[counter] == buffer[i]) {
            counter++;
            if (counter == sequence_size) 
                return i;
        }
        else 
            counter = 0;
    }
    return 0;
}
char check_seqence(char *sequence, int sequence_size, char *buffer)
{
    for (int i = 0; i < sequence_size; i++) {
        if (sequence[i] != buffer[i]) 
            return 1;
    }
    return 0;
}

static int read_file(char *file_name, unsigned char **file_content)
{
    FILE *file = fopen(file_name, "rb");
    if (!file) 
        ERR("Error: opening file");

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *file_content = malloc(file_size);
    if (fread(*file_content, 1, file_size, file) != file_size) {
        free(*file_content);
        ERR("Error: reading file");
    }

    fclose(file);
    return file_size;
}

