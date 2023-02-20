#ifndef __SPICE_SPRITE_H__
#define __SPICE_SPRITE_H__
#include <SDL2/SDL.h>
#include <spice_util.h>
#include <spice_math.h>
#include <spice_graphics.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include <nuklear.h>
#include <nuklear_sdl_gl3.h>

typedef struct {
    uint64_t _id;
    uint32_t _ref_count;
    uint16_t frame_w, frame_h;
    
    char* texture_data;
    unsigned int texture_id;

    struct nk_image image;
} sp_sprite;

typedef struct {
    uint32_t _next_free;
    uint32_t _sprite_count;
    uint32_t _in_draw;

    sp_sprite* sprites;
    struct nk_context* nk_ctx;
} sp_sprite_manager;

void spiceSpriteInit(uint32_t sprite_count);

sp_sprite* spiceLoadSprite(char* path);

void spiceFreeSprite(sp_sprite* sprite);

void spiceDrawSprite(sp_sprite* sprite, float x, float y, float rotation, uint32_t row, float frame);

void spiceSpriteClose();

void spiceSpriteBeginDraw();

void spiceSpriteEndDraw();

void spiceSpriteDrawText(char* text, float x, float y);

#endif