#ifndef __SPICE_GRAPHICS_H__
#define __SPICE_GRAPHICS_H__
#include <SDL2/SDL.h>
#include <SDL_gpu.h>

typedef struct {
    SDL_Window* window;
    GPU_Target* window_target;

    uint32_t target_fps, ticks_per_frame;
    uint64_t cur_time, prev_time;

} spice_graphics;

void spiceGraphicsInit(int width, int height, int target_fps, int window_flags);

GPU_Target* spiceGraphicsWindowTarget();

void spiceGraphicsClose();

void spiceGraphicsDraw();

void spiceGraphicsStep();

#endif