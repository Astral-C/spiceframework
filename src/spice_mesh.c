#include <spice_mesh.h>
#include <spice_graphics.h>

static spice_mesh_manager mesh_manager = {0};

const char* default_vtx_shader_source = "\
    #version 450\
    #extension GL_ARB_separate_shader_objects : enable\
    \
    uniform mat4 gpu_ModelViewProjectionMatrix;\
    \
    layout(location = 0) in vec3 inPosition;\
    layout(location = 1) in vec3 inNormal;\
    layout(location = 2) in vec2 inTexCoord;\
    layout(location = 3) in vec4 inColor;\
    \
    layout(location = 0) out vec3 fragNormal;\
    layout(location = 1) out vec2 fragTexCoord;\
    layout(location = 2) out vec4 fragColori;\
    \
    void main()\
    {\
        fragNormal = normalize(inNormal.xyz);\
        gl_Position = gpu_ModelViewProjectionMatrix * vec4(inPosition, 1.0);\
        fragColori = inColor;\
    }\
";

/*    
    #version 120\
    attribute vec3 gpu_Vertex;\
    attribute vec4 gpu_Color;\
    uniform mat4 gpu_ModelViewProjectionMatrix;\
    varying vec4 color;\
    void main(void)\
    {\
        color = gpu_Color;\
        gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 1.0);\
    }\
";
 */


const char* default_frg_shader_source = "\
    #version 450\
    #extension GL_ARB_separate_shader_objects : enable\
    \
    layout(binding = 0) uniform sampler2D texSampler;\
    layout(location = 0) in vec3 fragNormal;\
    layout(location = 1) in vec2 fragTexCoord;\
    layout(location = 2) in vec4 fragColori;\
    \
    layout(location = 0) out vec4 outColor;\
    \
    void main()\
    {\
        vec4 baseColor = texture(texSampler, fragTexCoord);\
        outColor = fragColori * baseColor;\
    }\
";

/*
    #version 120\
    varying vec4 color;\
    void main(void)\
    {\
        gl_FragColor = color;\
    }\
";
*/

void spiceMeshManagerCleanup(){
    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(mesh->_in_use){
            glDeleteVertexArrays(1, &mesh->_vao_id);
            glDeleteBuffers(1, &mesh->_vbo_id);
        } 
    }

    if(mesh_manager.meshes != NULL) free(mesh_manager.meshes);

    GPU_FreeShader(mesh_manager._default_shader);
}

void spiceMeshManagerInit(uint32_t mesh_max){
    mesh_manager.meshes = malloc(sizeof(sp_mesh) * mesh_max);
    memset(mesh_manager.meshes, 0, sizeof(sp_mesh) * mesh_max);
    mesh_manager.mesh_max = mesh_max;

    //set up default shader

    GLuint vs = GPU_LoadShader(GPU_VERTEX_SHADER, "assets/vtx.glsl");//default_vtx_shader_source);
    GLuint fs = GPU_LoadShader(GPU_FRAGMENT_SHADER, "assets/frg.glsl");
    mesh_manager._default_shader = GPU_LinkShaders(vs, fs);
    mesh_manager._mvp_loc = GPU_GetUniformLocation(mesh_manager._default_shader, "gpu_ModelViewProjectionMatrix");

    atexit(spiceMeshManagerCleanup);
}

sp_mesh* spiceMeshNewDynamic(uint32_t max_vertices){
    sp_mesh* new_mesh = NULL;
    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(!mesh->_in_use){
            mesh->_in_use = 1;
            mesh->_dynamic = 1;
            new_mesh = mesh;
            break;
        } 
    }

    if(new_mesh == NULL){
        spice_error("No meshes available!", NULL);
        return NULL;
    }
    
    new_mesh->vertex_count = max_vertices;
    new_mesh->vertices = malloc(sizeof(sp_vertex)*max_vertices);
    memset(new_mesh->vertices, 0, max_vertices);

    glGenVertexArrays(1, &new_mesh->_vao_id);
    glBindVertexArray(new_mesh->_vao_id);
    glGenBuffers(1, &new_mesh->_vbo_id);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, texcoord));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, color));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_vertex) * max_vertices, new_mesh->vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return new_mesh;
}

void spiceMeshUpdateBuffer(sp_mesh* mesh){
    if(mesh->_dynamic != 1){
        spice_error("Attempted to update static mesh",NULL);
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_vbo_id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sp_vertex) * mesh->vertex_count, mesh->vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void spiceMeshSetVertex(sp_mesh* mesh, sp_vertex vtx, uint32_t idx){
    if(!mesh->_dynamic){
        spice_error("Attempted to set vertex on static model", NULL);
        return;
    }

    memcpy(&mesh->vertices[idx], &vtx, sizeof(sp_vertex));
}

void spiceMeshFree(sp_mesh* mesh){
    mesh->_in_use = 0;
    glDeleteBuffers(1, &mesh->_vbo_id);
    free(mesh->vertices);
}


void spiceMeshManagerDraw(){

    float mvp[16];

    GPU_FlushBlitBuffer();
    GPU_MatrixMode(spiceGraphicsWindowTarget(), GPU_MODEL);
    GPU_PushMatrix();
    GPU_LoadIdentity();
    GPU_MatrixMode(spiceGraphicsWindowTarget(), GPU_VIEW);
    GPU_PushMatrix();
    GPU_LoadIdentity();
    GPU_MatrixMode(spiceGraphicsWindowTarget(), GPU_PROJECTION);
    GPU_PushMatrix();
    GPU_LoadIdentity();

    GPU_GetModelViewProjection(mvp);

    glUseProgram(mesh_manager._default_shader);

    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(!mesh->_in_use) continue;
        if(mesh->texture != NULL) glBindTexture(GL_TEXTURE_2D, GPU_GetTextureHandle(mesh->texture));
        glBindVertexArray(mesh->_vao_id);
        glUniformMatrix4fv(mesh_manager._mvp_loc, 1, 0, mvp);
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
    }
    
    GPU_ResetRendererState();
    GPU_MatrixMode(spiceGraphicsWindowTarget(), GPU_MODEL);
    GPU_PopMatrix();
    GPU_MatrixMode(spiceGraphicsWindowTarget(), GPU_VIEW);
    GPU_PopMatrix();
    GPU_MatrixMode(spiceGraphicsWindowTarget(), GPU_PROJECTION);
    GPU_PopMatrix();

}