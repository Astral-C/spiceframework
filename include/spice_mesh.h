#ifndef __SPICE_MESH_H__
#define __SPICE_MESH_H__
#include <SDL_gpu_OpenGL_3.h>
#include <spice_math.h>
#include <spice_util.h>

typedef struct {
    sp_vec3 position;
    sp_vec3 normal;
    sp_vec2 texcoord;
    sp_vec4 color;
} sp_vertex;

typedef struct {
    uint8_t _in_use;
    uint8_t _dynamic;
    GLuint _vao_id;
    GLuint _vbo_id;
    GLuint _instance_id;

    uint32_t _instance_count;

    uint32_t vertex_count;
    sp_vertex* vertices;
    GPU_Image* texture;
} sp_mesh;

typedef struct {
    GLuint _default_shader;
    GLuint _mvp_loc;
    
    uint32_t mesh_max;
    sp_mesh* meshes;
} spice_mesh_manager;

void spiceMeshManagerInit(uint32_t mesh_max);

sp_mesh* spiceMeshLoadCinnamodel(char* model_path);
sp_mesh* spiceMeshNew(uint32_t vertex_count);
sp_mesh* spiceMeshNewDynamic(uint32_t vertex_count);

void spiceMeshUpdateBuffer(sp_mesh* mesh);
void spiceMeshSetVertex(sp_mesh* mesh, sp_vertex vtx, uint32_t idx);

void spiceMeshFree(sp_mesh* mesh);

void spiceMeshManagerDraw();

extern sp_vec3 cam_position;
extern sp_vec3 cam_rotation;

#endif