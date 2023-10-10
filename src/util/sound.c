#include <stdio.h>
#include <stdlib.h>

#ifdef linux

#include <signal.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#else

#include <assert.h>
#include <windows.h>
#include <mmsystem.h>
#endif

#include "sound.h"

#define ERR(msg, ...) {fprintf(stderr, msg); return -1;}

static int read_file(char *file_name, unsigned char **file_content);
char check_seqence(char *sequence, int sequence_size, char *buffer);
char search_sequence(char *sequence, int sequence_size, char *buffer, int buffer_size);

int read_riff_chunk(wav_file_info_t *info);
int read_fmt_subchunk(wav_file_info_t *info);
int read_data_subchunk(wav_file_info_t *info);
void sound_unload_file(sound_t sound);
int load_wav_file(char *file, wav_file_info_t *info);

int sound_free(sound_t sound[]) 
{
    for (int i = 0; i < SOUND_MAX; i++) 
    {
        sound_kill(sound[i]);
        sound_unload_file(sound[i]);
    }
    return 0;
}

#ifdef linux
void sound_unload_file(sound_t sound)
{
    free(sound.info.file_data);
}

int sound_is_alive(sound_t sound) 
{
    return !waitpid(sound.pid, NULL, WNOHANG);
}

void sound_kill(sound_t sound) 
{
    if (!sound.pid) 
        return;
    kill(sound.pid, SIGINT); 
}

snd_pcm_t *handle;
void *global_sound;
void signal_handle(int signal)
{
    if (SIGINT == signal) 
    {
        sound_t sound = *(sound_t*) global_sound;
        sound_unload_file(sound);
        snd_pcm_drop(handle);
        /** snd_pcm_close(handle); */
        exit(1);
    }
}

int sound_play(sound_t *sound)
{
    if (sound_is_alive(*sound)) 
        sound_kill(*sound);

    sound->pid = fork(); 
    if (sound->pid < 0) 
        return 1;

    if (sound->pid) 
        return 0;

    global_sound = sound;
    signal(SIGINT, signal_handle);

    snd_pcm_hw_params_t *params;
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        perror("Error: opening PCM device\n");
        exit(1);
    }

    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, sound->info.num_channels);

    unsigned int rate = sound->info.sample_rate;
    snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);

    if (snd_pcm_hw_params(handle, params) < 0)
    {
        perror("Error: opening PCM device\n");
        exit(1);
    }

    snd_pcm_hw_params_free(params);
    snd_pcm_prepare(handle);

    snd_pcm_writei(handle, sound->info.data, sound->info.num_samples);
    snd_pcm_drain(handle);

    sound_unload_file(*sound);
    snd_pcm_close(handle);
    exit(1);
}

int sound_init(sound_t *sound)
{
    if (load_wav_file("resources/launch.wav", 
                      &sound[SOUND_LAUNCH].info)) 
        ERR("Error: loading launch.wav file");

    if (load_wav_file("resources/shoot.wav", 
                      &sound[SOUND_SHOOT].info)) 
        ERR("Error: loading shoot.vav file");

    if (load_wav_file("resources/break.wav", 
                       &sound[SOUND_BREAK].info)) 
        ERR("Error: loading break.wav file");

    if (load_wav_file("resources/game_over.wav",
                      &sound[SOUND_GAME_OVER].info))
        ERR("Error: loading game_over.wav file");


    if (load_wav_file("resources/select.wav",
                &sound[SOUND_SELECT].info))
        ERR("Error: loading select.wav file");

}
#else
int sound_is_alive(sound_t sound) 
{
    if (sound.header.dwFlags == WHDR_PREPARED)
        return 0;

    return !(sound.header.dwFlags & WHDR_DONE);
}

int sound_play(sound_t *sound)
{
    if (sound_is_alive(*sound)) 
        sound_kill(*sound);

    waveOutWrite(sound->dev, &sound->header, sizeof(sound->header));
}

void sound_unload_file(sound_t sound)
{
    free(sound.info.file_data);
}

void sound_kill(sound_t sound)
{
    waveOutReset(sound.dev);
}

sound_t sound_load_file(char *file)
{
    sound_t sound;
    assert(load_wav_file(file, &sound.info) == 0);

    WAVEFORMATEX wavFormat = {
        .wFormatTag = WAVE_FORMAT_PCM,
        .nChannels = sound.info.num_channels,
        .nSamplesPerSec = sound.info.sample_rate,
        .wBitsPerSample = sound.info.bits_per_sample,
        .nBlockAlign = sound.info.block_align,
        .nAvgBytesPerSec = sound.info.byte_rate,
        .cbSize = 0
    };

    assert(waveOutOpen(&sound.dev, WAVE_MAPPER, &wavFormat, 0, 0, CALLBACK_NULL) 
            == MMSYSERR_NOERROR );

    WAVEHDR header = {
        .lpData = sound.info.data,
        .dwBufferLength = sound.info.data_size,
        .dwBytesRecorded = 0,
        .dwFlags = 0,
        .dwLoops = 0
    };

    sound.header = header;
    assert(waveOutPrepareHeader(sound.dev, &sound.header, sizeof(WAVEHDR)) 
            == MMSYSERR_NOERROR);

    return sound;
}

int sound_init(sound_t *sound)
{
    if (waveOutGetNumDevs() == 0)
        ERR("No audio devices available\n");

    sound[SOUND_LAUNCH]     = sound_load_file("resources/launch.wav");
    sound[SOUND_SHOOT]      = sound_load_file("resources/shoot.wav");
    sound[SOUND_BREAK]      = sound_load_file("resources/break.wav");
    sound[SOUND_GAME_OVER]  = sound_load_file("resources/game_over.wav");
    sound[SOUND_SELECT]     = sound_load_file("resources/select.wav");
}
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
    info->num_samples = info->data_size / (info->num_channels * (info->bits_per_sample/8));

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

