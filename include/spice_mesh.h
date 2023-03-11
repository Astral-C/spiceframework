#ifndef __SPICE_MESH_H__
#define __SPICE_MESH_H__
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <spice_math.h>
#include <spice_util.h>
#include <spice_cam.h>
#include <spice_texture.h>

///////////////////
////
//// Regular Mesh Renderer
////
///////////////////

typedef struct {
    sp_vec3 position;
    sp_vec3 normal;
    sp_vec2 texcoord;
    sp_vec4 color;
    sp_vec2 texcoord_scale;
    uint32_t texture;
} sp_vertex;

typedef struct {
    uint8_t _in_use;
    uint8_t _dynamic;

    GLuint _vao_id;
    GLuint _ebo_id;
    GLuint _vbo_id;

    GLuint _instance_id;

    uint32_t _instance_count;

    uint32_t vertex_count;
    uint32_t index_count;

    uint32_t* indices;
    sp_vertex* vertices;
    sp_texture* texture;
    
    tm_mat4 transform;
} sp_mesh;

typedef struct {
    GLuint _default_shader;
    GLuint _mvp_loc;
    
    uint32_t mesh_max;
    sp_mesh* meshes;
} spice_mesh_manager;

///////////////////
////
//// Billboard/Point Sprite renderer
////
///////////////////

typedef struct {
    uint8_t _in_use;
    sp_vec3 position;
    int32_t sprite_size;
    int32_t texture;
    int32_t size_fixed;
    int32_t flip;
} sp_point_sprite;

typedef struct {
    GLuint _ps_shader;
    GLuint _mvp_loc;
    sp_texture* textures; //3d texture, each layer is different point sprite texture
    
    GLuint _vao_id;
    GLuint _vbo_id;
    
    uint32_t ps_max;
    sp_point_sprite* points;
} spice_point_sprite_manager;

///////////////////
////
//// Regular Mesh Renderer Functions
////
///////////////////

void spiceMeshManagerInit(uint32_t mesh_max);

sp_mesh* spiceMeshLoadCinnamodel(char* model_path);
sp_mesh* spiceMeshNew(uint32_t vertex_count);
sp_mesh* spiceMeshNewDynamic(uint32_t vertex_count);

void spiceMeshUpdateBuffer(sp_mesh* mesh);
void spiceMeshSetVertex(sp_mesh* mesh, sp_vertex vtx, uint32_t idx);

void spiceMeshFree(sp_mesh* mesh);

void spiceMeshManagerDraw();

///////////////////
////
//// Point Sprite Functions
////
///////////////////

void spicePointSpritesInit(uint32_t ps_max, uint32_t texture_count, uint32_t max_texture_res);

void spicePointSpriteSetTexture(uint8_t idx, char* img);

sp_point_sprite* spicePointSpriteNew();

void spicePointSpriteFree(sp_point_sprite* sprite);

void spicePointSpriteDraw();

#endif