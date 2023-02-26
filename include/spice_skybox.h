#ifndef __SPICE_SKYBOX_H__
#define __SPICE_SKYBOX_H__
#include <SDL2/SDL.h>
#include <spice_util.h>
#include <spice_math.h>
#include <spice_graphics.h>
#include <spice_texture.h>

typedef struct {
    GLuint _vbo, _vao, _shader;
    sp_texture* texture;
} sp_skybox;

sp_skybox* spiceSkyboxNew();
void spiceSkyboxRender(sp_skybox* skybox);
void spiceSkyboxFree();

#endif