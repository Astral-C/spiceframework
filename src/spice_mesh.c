#include <spice_mesh.h>
#include <spice_graphics.h>

static spice_mesh_manager mesh_manager = {0};

sp_vec3 cam_position = {0};
sp_vec3 cam_rotation = {0};

static char glErrorLogBuffer[4096];

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
        vec4 baseColor = texture(texSampler, fragTexCoord);\n\
        outColor = baseColor * fragColori;//vec4(1.0, 1.0, 1.0, 1.0);\n\
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
    

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, (const char**)&default_vtx_shader_source, NULL);
    glShaderSource(fs, 1, (const char**)&default_frg_shader_source, NULL);
    
    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint infoLogLength;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        glGetShaderInfoLog(vs, infoLogLength, NULL, glErrorLogBuffer);
        
        spice_error("Compile failure in vertex shader:\n%s\n", glErrorLogBuffer);
    }

    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE){
        GLint infoLogLength;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
        
        glGetShaderInfoLog(fs, infoLogLength, NULL, glErrorLogBuffer);
        
        spice_error("Compile failure in fragment shader:\n%s\n", glErrorLogBuffer);
    }

    mesh_manager._default_shader = glCreateProgram();
    
    glAttachShader(mesh_manager._default_shader, vs);
    glAttachShader(mesh_manager._default_shader, fs);

    glLinkProgram(mesh_manager._default_shader);
 
    glGetProgramiv(mesh_manager._default_shader, GL_LINK_STATUS, &status); 
    if(GL_FALSE == status) {
        GLint logLen; 
        glGetProgramiv(mesh_manager._default_shader, GL_INFO_LOG_LENGTH, &logLen); 
        glGetProgramInfoLog(mesh_manager._default_shader, logLen, NULL, glErrorLogBuffer); 
        spice_error("Shader Program Linking Error:\n%s\n", glErrorLogBuffer);
    } 

    glDetachShader(mesh_manager._default_shader, vs);
    glDetachShader(mesh_manager._default_shader, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

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
        spice_error("No meshes available!", NULL);
        return NULL;
    }
    
    new_mesh->vertex_count = max_vertices;
    new_mesh->vertices = malloc(sizeof(sp_vertex)*max_vertices);
    memset(new_mesh->vertices, 0, max_vertices);

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
            mesh->_dynamic = 1;
            new_mesh = mesh;
            break;
        } 
    }

    if(new_mesh == NULL){
        spice_error("No meshes available!", NULL);
        return NULL;
    }

    // Model Loader
    FILE* model = fopen(model_path, "rb");

    if(model == NULL){
        spice_error("Couldn't Open Model %s\n", model_path);
        return NULL;
    }

    uint32_t mesh_count = 0;

    spice_info("Reading Mesh Count\n", NULL);

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
    glDeleteVertexArrays(1, &mesh->_vao_id);
    glDeleteBuffers(1, &mesh->_vbo_id);
    free(mesh->vertices);
}


void spiceMeshManagerDraw(){
    float mvp[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glUseProgram(0);
    glBindVertexArray(0);

    glUseProgram(mesh_manager._default_shader);

    for (sp_mesh* mesh = mesh_manager.meshes; mesh < mesh_manager.meshes + mesh_manager.mesh_max; mesh++){
        if(!mesh->_in_use) continue;
        if(mesh->texture != NULL){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh->texture->texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        
        glBindVertexArray(mesh->_vao_id);

        glUniformMatrix4fv(mesh_manager._mvp_loc, 1, 0, mvp);
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
        
        glBindVertexArray(0);
    }

}