#ifndef __SPICE_MESH_H__
#define __SPICE_MESH_H__
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <spice_math.h>
#include <spice_util.h>
#include <spice_cam.h>

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
} sp_vertex;

typedef struct {
    uint8_t _in_use;
    uint32_t _ref_count; 
    GLuint texture;

    char* texture_data;
} sp_texture;

typedef struct {
    uint8_t _in_use;
    uint8_t _dynamic;

    GLuint _vao_id;
    GLuint _vbo_id;

    GLuint _instance_id;

    uint32_t _instance_count;

    uint32_t vertex_count;
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
} sp_point_sprite;

typedef struct {
    GLuint _ps_shader;
    GLuint _mvp_loc;
    GLuint textures; //3d texture, each layer is different point sprite texture
    
    GLuint _vao_id;
    GLuint _vbo_id;

    sp_texture sprite;

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

void spicePointSpritesInit(uint32_t ps_max);//, uint32_t texture_count, uint32_t max_texture_width, uint32_t max_texture_height);

void spicePointSpriteSetTexture(GLuint texture);

sp_point_sprite* spicePointSpriteNew();

void spicePointSpriteFree(sp_point_sprite* sprite);

void spicePointSpriteDraw();

///////////////////
////
//// Garbage Temporary Stuff, TODO: Remove this and give each renderer a pointer to the camera?
////
///////////////////

extern tm_orbit_camera camera;

#endif