#include <spice_util.h>
#include <spice_texture.h>
#include <stb_image.h>

static sp_texture_manager texture_manager = {0};

void spiceTextureDeleteData(sp_texture* texture){
    glDeleteTextures(1, &texture->texture_id); // Delete the opengl texture
}

void spiceTextureCleanup(){
    for (size_t t = 0; t < texture_manager.texture_max; t++)
    {
        spiceTextureDeleteData(&texture_manager.textures[t]);
    }

    free(texture_manager.textures);
}

void spiceTextureManagerInit(uint32_t texture_count){
    texture_manager._initialized = 1;
    texture_manager.texture_max = texture_count;
    texture_manager.textures = (sp_texture*)malloc(sizeof(sp_texture) * texture_count);

    memset(texture_manager.textures, 0, sizeof(sp_texture) * texture_count);
    atexit(spiceTextureCleanup);
}

sp_texture* spiceTextureArrayInit(uint32_t max_textures, uint32_t max_width, uint32_t max_height){
    sp_texture* found_texture = NULL;
    for (size_t i = 0; i < texture_manager.texture_max; i++){
        if(texture_manager.textures[i]._ref_count == 0){
            found_texture = &texture_manager.textures[i];
        }
    }

    found_texture->image_count = max_textures;

    glGenTextures(1, &found_texture->texture_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, found_texture->texture_id);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, max_width, max_height, max_textures, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    found_texture->_ref_count++;
    found_texture->_bind_point = GL_TEXTURE_2D_ARRAY;

    return found_texture;
}

void spiceTextureArrayLoad(sp_texture* texture, char* path, char idx){
    if(texture == NULL) return;

    if(idx > texture->image_count) return;

    int w, h, channels;
    unsigned char* img = stbi_load(path, &w, &h, &channels, 4);

    if(img == NULL){
        spice_error("Couldn't load texture %s\n", path);
        return;
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->texture_id);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, idx, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    stbi_image_free(img);
}

sp_texture* spiceTextureLoadCubeMap(char* paths[6]){
    //TODO: include all faces in this hash calc?
    uint64_t id = spHash(paths[0], strlen(paths[0])) + spHash(paths[1], strlen(paths[1])) + spHash(paths[2], strlen(paths[2]));
    sp_texture* found_texture = NULL;
    for (size_t i = 0; i < texture_manager.texture_max; i++){
        if(texture_manager.textures[i]._id == id){ // Sprite already loaded, return it. Or a collision. Oop-
            texture_manager.textures[i]._ref_count++;
            return &texture_manager.textures[i];
        }

        if(texture_manager.textures[i]._ref_count == 0){
            found_texture = &texture_manager.textures[i];
        }
    }

    // Out of texture space
    if(found_texture == NULL){
        spice_error("Out of texture space!\n");
        return NULL;
    }

    found_texture->_id = id;
    found_texture->image_count = 6;
    found_texture->_bind_point = GL_TEXTURE_CUBE_MAP;

    glGenTextures(1, &found_texture->texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, found_texture->texture_id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //no proper size since cubemap
    found_texture->width = -1;
    found_texture->height = -1;

    for (size_t face = 0; face < 6; face++){
        // Try to load the image...
        int w, h, channels;
        unsigned char* img = stbi_load(paths[face], &w, &h, &channels, 4);
                
        if(img == NULL){
            spice_error("STBI Couldn't load image %s!\n", paths[face]);
            return NULL; //and I oop- can't load the sprite
        }
            
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

        stbi_image_free(img);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    found_texture->_ref_count++;

    return found_texture;

}

sp_texture* spiceTexture2DLoad(char* path){
    uint64_t id = spHash(path, strlen(path));

    sp_texture* found_texture = NULL;
    for (size_t i = 0; i < texture_manager.texture_max; i++){
        if(texture_manager.textures[i]._id == id){ // Sprite already loaded, return it. Or a collision. Oop-
            texture_manager.textures[i]._ref_count++;
            return &texture_manager.textures[i];
        }
        
        if(texture_manager.textures[i]._ref_count == 0){
            found_texture = &texture_manager.textures[i];
        }
    }

    // Out of texture space
    if(found_texture == NULL){
        spice_error("Out of texture space!\n");
        return NULL;
    }

    // Try to load the image...
    int w, h, channels;
    unsigned char* img = stbi_load(path, &w, &h, &channels, 4);
            
    if(img == NULL){
        spice_error("STBI Couldn't load image!\n");
        return NULL; //and I oop- can't load the sprite
    }
    
    // Finalize setup and then return

    found_texture->_ref_count++;

    found_texture->_id = id;

    found_texture->width = w;
    found_texture->height = h;
    spice_info("Created Texture 2D %p\n", found_texture);

    glGenTextures(1, &found_texture->texture_id);

    glBindTexture(GL_TEXTURE_2D, found_texture->texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    found_texture->_bind_point = GL_TEXTURE_2D;

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(img);

    return found_texture;
}

void spiceTextureFree(sp_texture* texture){
    if(texture == NULL) return;
    texture->_ref_count--;


    if(texture->_ref_count == 0){
        spiceTextureDeleteData(texture);
        memset(texture, 0, sizeof(sp_texture));
    }
}


void spiceTextureBind(sp_texture* texture, GLuint tex_unit){
    glActiveTexture(tex_unit);
    glBindTexture(texture->_bind_point, texture->texture_id);
}

void spiceTextureUnbind(sp_texture* texture){
    glBindTexture(texture->_bind_point, 0);
}