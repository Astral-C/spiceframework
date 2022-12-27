#ifndef __SPICE_GRAPHICS_H__
#define __SPICE_GRAPHICS_H__
#include <SDL2/SDL.h>
#include <SDL_gpu.h>
#include <spice_collision.h>

typedef struct {
    SDL_Window* window;
    GPU_Target* window_target;
    GPU_Camera camera;

    uint32_t target_fps, ticks_per_frame;
    uint64_t cur_time, prev_time;
    float time;
} spice_graphics;

void spiceGraphicsInit(int width, int height, int target_fps, int window_flags);

GPU_Target* spiceGraphicsWindowTarget();

void spiceGraphicsClose();

void spiceGraphicsDraw();

void spiceGraphicsStep();

void spiceGraphicsSetCamera(float x, float y, float zoom, float angle);

void spiceGraphicsScreenMod(sp_vec4 color);

#endif