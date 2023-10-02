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
} wav_file_info_t;

typedef struct {
    wav_file_info_t info;
    int source;
    int buffer;
} sound_element_t;

typedef struct {
    void *context;
    void *device;
    sound_element_t sounds[SOUND_MAX];
} sounds_t;

int free_sound(sounds_t sound);
void sound_play(sounds_t sound, enum sounds_enum i);
void sound_pause(sounds_t sound, enum sounds_enum i);
char isSoundPlaying(sounds_t sound, enum sounds_enum i);
sounds_t sound_init();
