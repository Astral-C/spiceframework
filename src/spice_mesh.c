#include <spice_mesh.h>
#include <spice_graphics.h>
#include <tm_matrix.h>
#include <tm_vector.h>
#include <stb_image.h>

static spice_mesh_manager mesh_manager = {0};
static spice_point_sprite_manager point_sprite_manager = {0};

tm_orbit_camera camera = {0};

const char* default_vtx_shader_source = "#version 330\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform mat4 gpu_ModelViewProjectionMatrix;\n\
    uniform mat4 gpu_transform;\n\
    \
    layout(location = 0) in vec3 inPosition;\n\
    layout(location = 1) in vec3 inNormal;\n\
    layout(location = 2) in vec2 inTexCoord;\n\
    layout(location = 3) in vec4 inColor;\n\
    layout(location = 4) in vec2 inTexCoordScale;\n\
    layout(location = 5) in int tex;\n\
    \
    layout(location = 0) out vec3 fragNormal;\n\
    layout(location = 1) out vec2 fragTexCoord;\n\
    layout(location = 2) out vec4 fragColori;\n\
    layout(location = 3) out vec2 fragTexCoordScale;\n\
    layout(location = 4) flat out int fragTextureIdx;\n\
    \
    void main()\n\
    {\
        gl_Position = gpu_ModelViewProjectionMatrix * gpu_transform * vec4(inPosition, 1.0);\n\
        fragNormal = normalize(inNormal.xyz);\n\
        fragColori = inColor;\n\
        fragTexCoord = inTexCoord;\n\
        fragTexCoordScale = inTexCoordScale;\n\
        fragTextureIdx = tex;\n\
    }\
";

const char* default_frg_shader_source = "#version 330\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform sampler2DArray texSampler;\n\
    layout(location = 0) in vec3 fragNormal;\n\
    layout(location = 1) in vec2 fragTexCoord;\n\
    layout(location = 2) in vec4 fragColori;\n\
    layout(location = 3) in vec2 fragTexCoordScale;\n\
    layout(location = 4) flat in int fragTextureIdx;\n\
    \n\
    layout(location = 0) out vec4 outColor;\n\
    \
    void main()\n\
    {\n\
        vec4 baseColor = texture(texSampler, vec3(mod(fragTexCoord.x, fragTexCoordScale.x), mod(fragTexCoord.y, fragTexCoordScale.y), fragTextureIdx));\n\
        outColor = baseColor * fragColori;//vec4(1.0, 1.0, 1.0, 1.0);\n\
        if(baseColor.a < 1.0 / 255.0) discard;\n\
    }\
";

const char* default_ps_vtx_shader_source = "#version 330\n\
layout (location = 0) in vec3 position;\n\
layout (location = 1) in int tex;\n\
layout (location = 2) in int size;\n\
layout (location = 3) in int flip_tex;\n\
flat out int tex_idx;\n\
flat out int flip;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
void main()\n\
{\n\
    gl_Position = gpu_ModelViewProjectionMatrix * vec4(position, 1.0);\n\
    gl_PointSize = min(size, size / gl_Position.w);\n\
    tex_idx = tex;\n\
    flip = flip_tex;\n\
}\
";

const char* default_ps_frg_shader_source = "#version 330\n\
uniform sampler2DArray spriteTexture;\n\
flat in int tex_idx;\n\
flat in int flip;\n\
void main()\n\
{\n\
    if(flip == 0){\n\
        gl_FragColor = texture(spriteTexture, vec3(gl_PointCoord, tex_idx));\n\
    } else {\n\
        gl_FragColor = texture(spriteTexture, vec3(-gl_PointCoord.x, gl_PointCoord.y, tex_idx));\n\
    }\n\
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
            tm_mat4_identity(mesh->transform);
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
            tm_mat4_identity(mesh->transform);
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

    uint32_t mesh_count, texture_offset, texture_count, max_w, max_h;

    spice_info("Reading Mesh Count\n");

    fread(&texture_offset, sizeof(uint32_t), 1, model);
    fread(&mesh_count, sizeof(uint32_t), 1, model);

    spice_info("Read Mesh Count %d\nCollecting vertex and index count...\n", mesh_count);

    fread(&new_mesh->index_count, sizeof(uint32_t), 1, model);
    fread(&new_mesh->vertex_count, sizeof(uint32_t), 1, model);
    spice_info("Mesh has %d indices and %d vertices\n", new_mesh->index_count, new_mesh->vertex_count);

    spice_info("Allocating vertex  and indexbuffer\n",NULL);
    new_mesh->vertices = malloc(sizeof(sp_vertex) * new_mesh->vertex_count);
    new_mesh->indices = malloc(sizeof(uint32_t) * new_mesh->index_count);
    spice_info("Finished allocating vertex buffer\n",NULL);

    fread(new_mesh->indices, sizeof(uint32_t), new_mesh->index_count, model);
    fread(new_mesh->vertices, sizeof(sp_vertex), new_mesh->vertex_count, model);

    fseek(model, texture_offset, 0);
    fread(&texture_count, sizeof(uint32_t), 1, model);
    fread(&max_w, sizeof(uint32_t), 1, model);
    fread(&max_h, sizeof(uint32_t), 1, model);
    spice_info("Model has %u Textures, max width is %u max height is %u\n", texture_count, max_w, max_h);

    new_mesh->texture = spiceTextureArrayInit(texture_count, max_w, max_h);

    for (size_t i = 0; i < texture_count; i++){
        uint32_t data_size, w, h;

        fread(&data_size, sizeof(uint32_t), 1, model);
        fread(&w, sizeof(uint32_t), 1, model);
        fread(&h, sizeof(uint32_t), 1, model);

        spice_info("Reading %u bytes of image data for texture %ux%u\n", data_size, w, h);

        unsigned char* img_data = malloc(sizeof(uint32_t) * max_w * max_h);
        memset(img_data, 0, sizeof(uint32_t) * max_w * max_h);

        fread(img_data, data_size, 1, model);

        spiceTextureArrayLoadData(new_mesh->texture, w, h, img_data, i);

        free(img_data);
    }
    

    glGenVertexArrays(1, &new_mesh->_vao_id);
    glBindVertexArray(new_mesh->_vao_id);
 
    glGenBuffers(1, &new_mesh->_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sp_vertex) * new_mesh->vertex_count, new_mesh->vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &new_mesh->_ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_mesh->_ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * new_mesh->index_count, new_mesh->indices, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, texcoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, color));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(sp_vertex), (void*)offsetof(sp_vertex, texcoord_scale));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(sp_vertex), (void*)offsetof(sp_vertex, texture));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
        }
        
        glBindVertexArray(mesh->_vao_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_ebo_id);
    
        glUniformMatrix4fv(mesh_manager._mvp_loc, 1, 0, mvp);
        glUniformMatrix4fv(glGetUniformLocation(mesh_manager._default_shader, "gpu_transform"), 1, 0, mesh->transform);
        //glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, NULL);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(sp_point_sprite), (void*)offsetof(sp_point_sprite, flip));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    point_sprite_manager.textures = spiceTextureArrayInit(texture_count, max_texture_res, max_texture_res);

    atexit(spicePointSpriteCleanup);
}

void spicePointSpriteSetTexture(uint8_t idx, char* img){
    spiceTextureArrayLoad(point_sprite_manager.textures, img, idx);
}

void spicePointSpriteSetTextureFromMemory(uint8_t idx, char* data, size_t size){
    int w, h, channels;
    unsigned char* img = stbi_load_from_memory(data, size, &w, &h, &channels, 4);

    spiceTextureArrayLoadData(point_sprite_manager.textures, w, h, img, idx);

    stbi_image_free(img);
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
    memset(sprite, 0, sizeof(sp_point_sprite));
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