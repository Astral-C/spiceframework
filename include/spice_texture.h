#ifndef __SPICE_TEXTURE_H__
#define __SPICE_TEXTURE_H__
#include <SDL2/SDL.h>
#include <spice_util.h>
#include <spice_math.h>
#include <spice_graphics.h>

typedef struct {
    uint64_t _id;
    uint32_t _ref_count;
    GLuint _bind_point;
    uint32_t width, height, image_count;
    
    unsigned int texture_id;
} sp_texture;

typedef struct {
    uint32_t _initialized;
    uint32_t texture_max;
    sp_texture* textures;
} sp_texture_manager;


void spiceTextureManagerInit(uint32_t texture_count);

sp_texture* spiceTextureArrayInit(uint32_t max_textures, uint32_t max_width, uint32_t max_height);

void spiceTextureArrayLoad(sp_texture* texture, char* path, char idx);

void spiceTextureArrayLoadData(sp_texture* texture, int w, int h, char* img, char idx);

sp_texture* spiceTextureLoadCubeMap(char* paths[6]);

sp_texture* spiceTexture2DLoad(char* path);

void spiceTextureFree(sp_texture* texture);

void spiceTextureBind(sp_texture* texture, GLuint tex_unit);
void spiceTextureUnbind(sp_texture* texture);

#endif