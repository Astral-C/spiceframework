#include <spice_mesh.h>
#include <spice_graphics.h>
#include <tm_matrix.h>
#include <tm_vector.h>

static spice_mesh_manager mesh_manager = {0};
static spice_point_sprite_manager point_sprite_manager = {0};

tm_orbit_camera camera = {0};

const char* default_vtx_shader_source = "#version 330\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform mat4 gpu_ModelViewProjectionMatrix;\n\
    \
    layout(location = 0) in vec3 inPosition;\n\
    layout(location = 1) in vec3 inNormal;\n\
    layout(location = 2) in vec2 inTexCoord;\n\
    layout(location = 3) in vec4 inColor;\n\
    \
    layout(location = 0) out vec3 fragNormal;\n\
    layout(location = 1) out vec2 fragTexCoord;\n\
    layout(location = 2) out vec4 fragColori;\n\
    \
    void main()\n\
    {\
        gl_Position = gpu_ModelViewProjectionMatrix * vec4(inPosition, 1.0);\n\
        fragNormal = normalize(inNormal.xyz);\n\
        fragColori = inColor;\n\
        fragTexCoord = inTexCoord;\n\
    }\
";

const char* default_frg_shader_source = "#version 330\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform sampler2D texSampler;\n\
    layout(location = 0) in vec3 fragNormal;\n\
    layout(location = 1) in vec2 fragTexCoord;\n\
    layout(location = 2) in vec4 fragColori;\n\
    \n\
    layout(location = 0) out vec4 outColor;\n\
    \
    void main()\n\
    {\n\
        vec4 baseColor = texture(texSampler, vec2(fragTexCoord.x, -fragTexCoord.y));\n\
        outColor = baseColor * fragColori;//vec4(1.0, 1.0, 1.0, 1.0);\n\
    }\
";

const char* default_ps_vtx_shader_source = "#version 330\n\
layout (location = 0) in vec3 position;\n\
layout (location = 1) in int tex;\n\
layout (location = 2) in int size;\n\
layout (location = 3) in int fixed_size;\n\
flat out int tex_idx;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
void main()\n\
{\n\
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(position, 1.0);\n\
    if(fixed_size != 0){\n\
        gl_PointSize = size;\n\
    } else {\n\
        gl_PointSize = min(size, size / gl_Position.w);\n\
    }\n\
    tex_idx = tex;\n\
}\
";

const char* default_ps_frg_shader_source = "#version 330\n\
uniform sampler2DArray spriteTexture;\n\
flat in int tex_idx;\n\
void main()\n\
{\n\
    gl_FragColor = texture(spriteTexture, vec3(gl_PointCoord, tex_idx));\n\
    if(gl_FragColor.a < 1.0 / 255.0) discard;\n\
}\
";

void spiceMeshManagerCleanup(){
    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(mesh->_in_use){
            glDeleteVertexArrays(1, &mesh->_vao_id);
            glDeleteBuffers(1, &mesh->_vbo_id);
            free(mesh->vertices);
        } 
    }

    if(mesh_manager.meshes != NULL) free(mesh_manager.meshes);

    glDeleteProgram(mesh_manager._default_shader);
}

void spiceMeshManagerInit(uint32_t mesh_max){
    mesh_manager.meshes = malloc(sizeof(sp_mesh) * mesh_max);
    memset(mesh_manager.meshes, 0, sizeof(sp_mesh) * mesh_max);
    mesh_manager.mesh_max = mesh_max;

    if(spiceGraphicsCompileShader((const char**)&default_vtx_shader_source, (const char**)&default_frg_shader_source, &mesh_manager._default_shader) == SP_ERROR){
        spice_error("Failed to compile shader for mesh manager!");
    }

    mesh_manager._mvp_loc = glGetUniformLocation(mesh_manager._default_shader, "gpu_ModelViewProjectionMatrix");

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
        spice_error("No meshes available!");
        return NULL;
    }
    
    new_mesh->vertex_count = max_vertices;
    new_mesh->vertices = malloc(sizeof(sp_vertex)*max_vertices);
    memset(new_mesh->vertices, 0, sizeof(sp_vertex)*max_vertices);

    glGenVertexArrays(1, &new_mesh->_vao_id);
    glBindVertexArray(new_mesh->_vao_id);
 
    glGenBuffers(1, &new_mesh->_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_vertex) * max_vertices, new_mesh->vertices, GL_DYNAMIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, texcoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return new_mesh;
}

sp_mesh* spiceMeshLoadCinnamodel(char* model_path){
    sp_mesh* new_mesh = NULL;
    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(!mesh->_in_use){
            mesh->_in_use = 1;
            mesh->_dynamic = 0;
            new_mesh = mesh;
            break;
        } 
    }

    if(new_mesh == NULL){
        spice_error("No meshes available!");
        return NULL;
    }

    // Model Loader
    FILE* model = fopen(model_path, "rb");

    if(model == NULL){
        spice_error("Couldn't Open Model %s\n", model_path);
        return NULL;
    }

    uint32_t mesh_count = 0;

    spice_info("Reading Mesh Count\n");

    fread(&mesh_count, sizeof(uint32_t), 1, model);

    spice_info("Read Mesh Count %d\nCollecting vertex count...\n", mesh_count);

    for (size_t i = 0; i < mesh_count; i++){
        uint32_t vtx_count = 0;
        fread(&vtx_count, sizeof(uint32_t), 1, model);
        spice_info("Mesh %d has %d vertices\n", i, vtx_count);
        new_mesh->vertex_count += vtx_count;
        fseek(model, sizeof(sp_vertex) * vtx_count, SEEK_CUR);
    }

    rewind(model);
    spice_info("Allocating vertex buffer\n",NULL);
    new_mesh->vertices = malloc(sizeof(sp_vertex) * new_mesh->vertex_count);
    spice_info("Finished allocating vertex buffer\n",NULL);

    fread(&mesh_count, sizeof(uint32_t), 1, model);
    uint32_t vtx_offset = 0;
    for (size_t i = 0; i < mesh_count; i++){
        spice_info("Reading Vertex data for mesh %d\n", i);
        uint32_t vtx_count = 0;
        fread(&vtx_count, sizeof(uint32_t), 1, model);
        fread(new_mesh->vertices + vtx_offset, sizeof(sp_vertex), vtx_count, model);
        vtx_offset += vtx_count;
    }


    glGenVertexArrays(1, &new_mesh->_vao_id);
    glBindVertexArray(new_mesh->_vao_id);
 
    glGenBuffers(1, &new_mesh->_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_vertex) * new_mesh->vertex_count, new_mesh->vertices, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, texcoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    fclose(model);

    return new_mesh;
}

void spiceMeshUpdateBuffer(sp_mesh* mesh){
    if(mesh->_dynamic != 1){
        spice_error("Attempted to update static mesh");
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, mesh->_vbo_id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sp_vertex) * mesh->vertex_count, mesh->vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void spiceMeshSetVertex(sp_mesh* mesh, sp_vertex vtx, uint32_t idx){
    if(!mesh->_dynamic){
        spice_error("Attempted to set vertex on static model");
        return;
    }

    memcpy(&mesh->vertices[idx], &vtx, sizeof(sp_vertex));
}

void spiceMeshFree(sp_mesh* mesh){
    mesh->_in_use = 0;
    glDeleteVertexArrays(1, &mesh->_vao_id);
    glDeleteBuffers(1, &mesh->_vbo_id);
    free(mesh->vertices);
    if(mesh->texture) spiceTextureFree(mesh->texture);
}


void spiceMeshManagerDraw(){
    tm_mat4 cam;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_DEPTH_CLAMP);

    glEnable(GL_CULL_FACE);  
    glFrontFace(GL_CCW);

    tm_mat4 model, mvp;

    tm_mat4_identity(model);

    spiceOrbitCamGetMVP(model, mvp);


    glUseProgram(0);
    glBindVertexArray(0);

    glUseProgram(mesh_manager._default_shader);

    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(!mesh->_in_use) continue;
        if(mesh->texture != NULL){
            spiceTextureBind(mesh->texture, GL_TEXTURE0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        
        glBindVertexArray(mesh->_vao_id);

        glUniformMatrix4fv(mesh_manager._mvp_loc, 1, 0, mvp);
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
        
        glBindVertexArray(0);
    }

}

///////////////////
////
//// Point Sprite Functions
////
///////////////////

void spicePointSpriteCleanup(){
    spiceTextureFree(point_sprite_manager.textures);
    glDeleteVertexArrays(1, &point_sprite_manager._vao_id);
    glDeleteBuffers(1, &point_sprite_manager._vbo_id);
    free(point_sprite_manager.points);
    
}

void spicePointSpritesInit(uint32_t ps_max, uint32_t texture_count, uint32_t max_texture_res){
    point_sprite_manager.ps_max = ps_max;

    point_sprite_manager.points = malloc(sizeof(sp_point_sprite)*ps_max);

    memset(point_sprite_manager.points, 0, sizeof(sp_point_sprite)*ps_max);

    spiceGraphicsCompileShader((const char**)&default_ps_vtx_shader_source, (const char**)&default_ps_frg_shader_source, &point_sprite_manager._ps_shader);

    point_sprite_manager._mvp_loc = glGetUniformLocation(point_sprite_manager._ps_shader, "gpu_ModelViewProjectionMatrix");

    glGenVertexArrays(1, &point_sprite_manager._vao_id);
    glBindVertexArray(point_sprite_manager._vao_id);
 
    glGenBuffers(1, &point_sprite_manager._vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, point_sprite_manager._vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_point_sprite) * ps_max, point_sprite_manager.points, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sp_point_sprite), (void*)offsetof(sp_point_sprite, position));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(sp_point_sprite), (void*)offsetof(sp_point_sprite, texture));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(sp_point_sprite), (void*)offsetof(sp_point_sprite, sprite_size));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(sp_point_sprite), (void*)offsetof(sp_point_sprite, size_fixed));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    point_sprite_manager.textures = spiceTextureArrayInit(texture_count, max_texture_res, max_texture_res);

    atexit(spicePointSpriteCleanup);
}

void spicePointSpriteSetTexture(uint8_t idx, char* img){
    spiceTextureArrayLoad(point_sprite_manager.textures, img, idx);
}

sp_point_sprite* spicePointSpriteNew(){
    sp_point_sprite* new_ps = NULL;
    for (sp_point_sprite* ps = point_sprite_manager.points; ps < point_sprite_manager.points + point_sprite_manager.ps_max; ps++){
        if(!ps->_in_use){
            ps->_in_use = 1;
            new_ps = ps;
            break;
        } 
    }

    if(new_ps == NULL){
        spice_error("No point sprites available!");
        return NULL;
    }

    return new_ps;
}

void spicePointSpriteFree(sp_point_sprite* sprite){
    sprite->_in_use = 0;
}

void spicePointSpriteDraw(){
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);

    tm_mat4 model, mvp;

    tm_mat4_identity(model);

    spiceOrbitCamGetMVP(model, mvp);

    glUseProgram(0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, point_sprite_manager._vbo_id);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sp_point_sprite) * point_sprite_manager.ps_max, point_sprite_manager.points);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(point_sprite_manager._ps_shader);

    spiceTextureBind(point_sprite_manager.textures, GL_TEXTURE0);
        
    glBindVertexArray(point_sprite_manager._vao_id);

    glUniformMatrix4fv(point_sprite_manager._mvp_loc, 1, 0, mvp);
    glDrawArrays(GL_POINTS, 0, point_sprite_manager.ps_max);
    
    spiceTextureUnbind(point_sprite_manager.textures);
    glBindVertexArray(0);

}