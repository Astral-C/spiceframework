#ifndef __SPICE_SPRITE_H__
#define __SPICE_SPRITE_H__
#include <SDL2/SDL.h>
#include <spice_util.h>
#include <spice_math.h>

typedef struct {
    uint64_t _id;
    uint32_t _ref_count;
    uint16_t frame_w, frame_h;
    
    char* texture_data;
    unsigned int texture_id;
} sp_sprite;

typedef struct {
    uint32_t _next_free;
    uint32_t _sprite_count;
    
    sp_sprite* sprites;
} sp_sprite_manager;

void spiceSpriteInit(uint32_t sprite_count);

sp_sprite* spiceLoadSprite(char* path);

void spiceFreeSprite(sp_sprite* sprite);

void spiceDrawSprite(sp_sprite* sprite, float x, float y, float rotation, uint32_t row, float frame);

void spiceSpriteClose();

#endif