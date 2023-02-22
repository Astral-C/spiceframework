#include <spice_sprite.h>
#include <spice_util.h>
#include <stb_image.h>

static sp_sprite_manager sprite_manager = {0};

void spiceSpriteInit(uint32_t sprite_count){
    sprite_manager._sprite_count = sprite_count;
    sprite_manager.sprites = (sp_sprite*)malloc(sizeof(sp_sprite) * sprite_count);

    sprite_manager.nk_ctx = spiceGetNuklearContext();
    sprite_manager._in_draw = 0;

    memset(sprite_manager.sprites, 0, sizeof(sp_sprite) * sprite_count);
    atexit(spiceSpriteClose);
}

sp_sprite* spiceLoadSprite(char* path){
    uint64_t id = spHash(path, strlen(path));

    sp_sprite* found_sprite = NULL;
    for (size_t i = 0; i < sprite_manager._sprite_count; i++){
        if(sprite_manager.sprites[i]._id == id){ // Sprite already loaded, return it. Or a collision. Oop-
            sprite_manager.sprites[i]._ref_count++;
            spice_info("Returning Sprite %d, IDs match\n", NULL);
            return &sprite_manager.sprites[i];
        }
        
        if(sprite_manager.sprites[i]._ref_count == 0){
            found_sprite = &sprite_manager.sprites[i];
        }
    }

    // Out of sprite space
    if(found_sprite == NULL){
        spice_error("Out of sprite space!\n", NULL);
        return NULL;
    }

    // Try to load the image...
    int w, h, channels;
    unsigned char* img = stbi_load(path, &w, &h, &channels, 4);
            
    if(img == NULL){
        spice_error("STBI Couldn't load image!\n", NULL);
        return NULL; //and I oop- can't load the sprite
    }


    // We can load the sprite properly so now and only now do we finally remove this sprite image
    if(found_sprite->texture_data != NULL){
        glDeleteTextures(1, &found_sprite->texture_id);
        stbi_image_free(found_sprite->texture_data);
        found_sprite->texture_data = NULL;
    }
    
    // Finalize setup and then return

    found_sprite->_ref_count++;

    found_sprite->texture_data = img;
    found_sprite->_id = id;

    found_sprite->frame_w = w;
    found_sprite->frame_h = h;
    spice_info("Returning Sprite with addr %p\n", found_sprite);

    glGenTextures(1, &found_sprite->texture_id);
    glBindTexture(GL_TEXTURE_2D, found_sprite->texture_id);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, found_sprite->texture_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    found_sprite->image = nk_image_id((int)found_sprite->texture_id);

    return found_sprite;
}

void spiceFreeSprite(sp_sprite* sprite){
    if(sprite == NULL) return;
    sprite->_ref_count--;
}

void spiceDrawSprite(sp_sprite* sprite, float x, float y, float rotation, uint32_t row, float frame){
    if(sprite == NULL || !sprite_manager._in_draw) return;

    //use nuklear to handle 2d draws
    nk_draw_image(nk_window_get_canvas(sprite_manager.nk_ctx), (struct nk_rect){x, y, 64, 64}, &sprite->image, (struct nk_color){255,255,255,255});
    /*
    GPU_Rect src = (GPU_Rect){((uint32_t)frame) * sprite->frame_w, row * sprite->frame_h, sprite->frame_w, sprite->frame_h};
    GPU_BlitTransform(sprite->texture, &src, sprite_manager._window_target, x * sprite->parallax_factor.x, y * sprite->parallax_factor.y, rotation, 1, 1);
    */

    // TODO: Implement instanced rendering
}

void spiceSpriteDrawText(char* text, float x, float y){
    if(!sprite_manager._in_draw) return;

    nk_draw_text(nk_window_get_canvas(sprite_manager.nk_ctx), (struct nk_rect){x, y, 200, 200}, text, strlen(text), sprite_manager.nk_ctx->style.font, (struct nk_color){0,0,0,0}, (struct nk_color){255,255,255,255});
}

void spiceSpriteClose(){
    for (size_t i = 0; i < sprite_manager._sprite_count; i++){
        if(sprite_manager.sprites[i].texture_data != NULL){
            glDeleteTextures(1, &sprite_manager.sprites[i].texture_id);
            stbi_image_free(sprite_manager.sprites[i].texture_data);
        }
    }
    free(sprite_manager.sprites);
}

void spiceSpriteBeginDraw(){
    nk_style_push_style_item(sprite_manager.nk_ctx, &sprite_manager.nk_ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0,0,0,0)));
	nk_style_push_vec2(sprite_manager.nk_ctx, &sprite_manager.nk_ctx->style.window.padding, nk_vec2(0,0));

    nk_begin(sprite_manager.nk_ctx, "", nk_rect(0,0,1280,720), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND);
    sprite_manager._in_draw = 1;
}

void spiceSpriteEndDraw(){
    nk_end(sprite_manager.nk_ctx);

    nk_style_pop_vec2(sprite_manager.nk_ctx);
    nk_style_pop_style_item(sprite_manager.nk_ctx);
    
    sprite_manager._in_draw = 0;
}