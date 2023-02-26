#include <spice_skybox.h>
#include <spice_graphics.h>
#include <spice_cam.h>
#include <tm_matrix.h>
#include <tm_vector.h>

float cube_vertices[] = {
  -1.0f,  1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  
  -1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f, -1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,
  
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
   
  -1.0f, -1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f, -1.0f,  1.0f,
  -1.0f, -1.0f,  1.0f,
  
  -1.0f,  1.0f, -1.0f,
   1.0f,  1.0f, -1.0f,
   1.0f,  1.0f,  1.0f,
   1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f,  1.0f,
  -1.0f,  1.0f, -1.0f,
  
  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f, -1.0f,
   1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f,  1.0f,
   1.0f, -1.0f,  1.0f
};

const char* skybox_vtx_shader_source = "#version 330\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform mat4 gpu_ModelViewProjectionMatrix;\n\
    \
    layout(location = 0) in vec3 inPosition;\n\
    layout(location = 0) out vec3 fragTexCoord;\n\
    \
    void main()\n\
    {\
        gl_Position =  (gpu_ModelViewProjectionMatrix * vec4(inPosition, 1.0)).xyww;\n\
        fragTexCoord = inPosition;\n\
    }\
";

const char* skybox_frg_shader_source = "#version 330\n\
    #extension GL_ARB_separate_shader_objects : enable\n\
    \
    uniform samplerCube texSampler;\n\
    layout(location = 0) in vec3 fragTexCoord;\n\
    layout(location = 0) out vec4 outColor;\n\
    \
    void main()\n\
    {\n\
        outColor = texture(texSampler, fragTexCoord);\n\
    }\
";

sp_skybox* spiceSkyboxNew(){
    sp_skybox* skybox = (sp_skybox*)malloc(sizeof(sp_skybox));
    memset(skybox, 0, sizeof(sp_skybox));

    if(spiceGraphicsCompileShader((const char**)&skybox_vtx_shader_source, (const char**)&skybox_frg_shader_source, &skybox->_shader) == SP_ERROR){
        spice_error("Failed to compile skybox shader!\n");
    }

    glGenVertexArrays(1, &skybox->_vao);
    glBindVertexArray(skybox->_vao);

    glGenBuffers(1, &skybox->_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, skybox->_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return skybox;
}

void spiceSkyboxRender(sp_skybox* skybox){
    glDepthMask(GL_FALSE);

    glUseProgram(0);
    glBindVertexArray(0);

    glUseProgram(skybox->_shader);

    tm_mat4 mvp;

    spiceOrbitCamGetView(mvp);
    glUniformMatrix4fv(glGetUniformLocation(skybox->_shader, "gpu_ModelViewProjectionMatrix"), 1, 0, mvp);

    glBindVertexArray(skybox->_vao);
    spiceTextureBind(skybox->texture, GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

void spiceSkyboxFree(sp_skybox* skybox){
    glDeleteVertexArrays(1, &skybox->_vao);
    glDeleteBuffers(1, &skybox->_vbo);
    
    free(skybox);
}