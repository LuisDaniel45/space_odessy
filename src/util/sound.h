#pragma once
enum  sounds_enum {
    SOUND_LAUNCH,
    SOUND_SHOOT,
    SOUND_BREAK,
    SOUND_GAME_OVER,
    SOUND_SELECT,
    SOUND_MAX
}; 

typedef struct {
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    void *data;
    int data_size;
    void *file_data;
    int file_size;
    int num_samples;
} wav_file_info_t;

typedef struct {
    int pid;
    wav_file_info_t info;
} sound_t;

int sound_free(sound_t sound[]);
int sound_play(sound_t *sound);
void sound_kill(sound_t sound);
int sound_is_alive(sound_t sound);
int sound_init(sound_t *sound);
