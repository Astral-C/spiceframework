#ifndef __SPICE_MIXER_H__
#define __SPICE_MIXER_H__

#include <SDL_audio.h>

typedef enum {
    CLIP_TYPE_WAV,
    CLIP_TYPE_RAW
} sp_clip_type;

typedef struct {
    int _in_use;
    sp_clip_type type;
    uint8_t playing;
    SDL_AudioSpec source_spec;

    int sample_offset;
    int length;
    int pitch;
    float volume;

    void* data;
} sp_clip;

typedef struct {
    uint8_t ready;
    float volume;
    SDL_AudioSpec _target_spec;
    SDL_AudioSpec _device_spec;
    SDL_AudioDeviceID _device;

    int max_clips;
    sp_clip* clips;

    void (*custom_mix)(); // user specified function to run every time the mixer runs
} spice_mixer;


void spiceMixerInit(int clip_max);
sp_clip* spiceMixerLoadWav(char* path);
void spiceMixerPause(int pause);

#endif