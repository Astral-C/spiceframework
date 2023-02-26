#include <spice_util.h>
#include <spice_mixer.h>

static spice_mixer mixer = {0};

void spiceMixClip(sp_clip* clip, int16_t *stream, int len){
    if(clip == NULL) return;

    if(clip->sample_offset >= clip->length){
        clip->playing = clip->loop;
        clip->sample_offset = 0;
    }

    switch (clip->source_spec.format)
    {
    case AUDIO_F32:
        for (size_t i = 0; i < len; i+=2){
            stream[i] += (int16_t)((((float*)clip->data)[clip->sample_offset] * INT16_MAX) * clip->volume * mixer.volume);
            stream[i+1] += (int16_t)((((float*)clip->data)[clip->sample_offset+1] * INT16_MAX) * clip->volume * mixer.volume);
            clip->sample_offset += (clip->source_spec.freq / mixer._target_spec.freq) * clip->pitch;
        }
        break;
    case AUDIO_S16:
        for (size_t i = 0; i < len; i+=2){
            stream[i] += (int16_t)(((int16_t*)clip->data)[clip->sample_offset] * clip->volume * mixer.volume);
            stream[i+1] += (int16_t)(((int16_t*)clip->data)[clip->sample_offset+1] * clip->volume * mixer.volume);
            clip->sample_offset += (clip->source_spec.freq / mixer._target_spec.freq) * clip->pitch;
        }
        break;        
    default:
        for (size_t i = 0; i < len; i++){
            stream[i] = 0;
        }
        spice_error("Unknown sample format! Format is %u\n", clip->source_spec.format);
        break;
    }
}


void spiceMixerMix(void *userdata, uint8_t *stream, int len){
    memset(stream,0,len);
    if(mixer.ready == 1){
        //Mix Clips here
        for(sp_clip* clip = mixer.clips; clip < mixer.clips + mixer.max_clips; clip++){
            if(clip->_in_use && clip->playing){
                spiceMixClip(clip, (int16_t*)stream, (uint32_t)(len/2));
            }
        }
    }
}

void spiceMixerCleanup(){
    for(sp_clip* clip = mixer.clips; clip < mixer.clips + mixer.max_clips; clip++){
        if(!clip->_in_use) continue;
        if(clip->type == CLIP_TYPE_WAV) SDL_FreeWAV(clip->data);
        if(clip->type == CLIP_TYPE_RAW) free(clip->data);
    }
    free(mixer.clips);
    SDL_CloseAudioDevice(mixer._device);
}

void spiceMixerInit(int clip_max){
    mixer.max_clips = clip_max;
    mixer.clips = malloc(sizeof(sp_clip)*clip_max);
    memset(mixer.clips, 0, sizeof(sp_clip)*clip_max);

	SDL_AudioSpec target_format;
	target_format.freq = 44100;
	target_format.format = AUDIO_S16;
	target_format.channels = 2;
	target_format.samples = 4096;
	target_format.callback = spiceMixerMix;
	target_format.userdata = NULL;

    mixer.volume = 0.5;

    mixer._target_spec = target_format;
    mixer._device = SDL_OpenAudioDevice(NULL, 0, &mixer._target_spec, &mixer._device_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if(mixer._device < 0){
        spice_error("Error Initializing SDL Audio Device: %s\n", SDL_GetError());
    }

    mixer.ready = 1;
    atexit(spiceMixerCleanup);
}


sp_clip* spiceMixerLoadWav(char* path){
    for(sp_clip* clip = mixer.clips; clip < mixer.clips + mixer.max_clips; clip++){
        if(!clip->_in_use){
            SDL_AudioSpec* spec = SDL_LoadWAV(path, &clip->source_spec, (Uint8**)(&clip->data), &clip->length);
            if(spec == NULL){
                spice_error("Error loading WAV as clip %s\n", SDL_GetError());
                break;
            }
            spice_info("Loaded WAV %s with freq %d\n", path, clip->source_spec.freq);
            clip->_in_use = 1;
            clip->pitch = 2; //default pitch to 1
            clip->type = CLIP_TYPE_WAV;
            clip->length /= 2; //2 bytes per sample, 2 samples (left/right) per update
            return clip;
        }
    }
}

void spiceMixerPause(int pause){
    SDL_PauseAudioDevice(mixer._device, pause);
}