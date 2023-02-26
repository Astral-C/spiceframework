#ifndef __SPICE_GRAPHICS_H__
#define __SPICE_GRAPHICS_H__
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <spice_collision.h>


typedef struct {
    SDL_Window* window;
    SDL_Color clear_color;
    SDL_GLContext context;
    struct nk_context* nk;

    uint32_t target_fps, ticks_per_frame;
    uint64_t cur_time, prev_time;
    float time;
} spice_graphics;

void spiceGraphicsInit(char* window_name, int width, int height, int target_fps, int window_flags);

void spiceGraphicsSetClearColor(sp_vec4 color);

void spiceGraphicsClose();

void spiceGraphicsDraw();

void spiceGraphicsStep();

void spiceGraphicsSetFullscreen(uint8_t fullscreen, uint8_t update_resolution);

void spiceGraphicsSetResolution(int width, int height);

void spiceGraphicsSetCamera(float x, float y, float zoom, float angle);

void spiceGraphicsScreenMod(sp_vec4 color);

SDL_Window* spiceGraphicsGetWindow();

struct nk_context* spiceGetNuklearContext();

sp_status spiceGraphicsCompileShader(const char** vtx_shader_source, const char** frg_shader_source, GLuint* out);

#endif