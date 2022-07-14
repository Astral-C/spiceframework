#include <spice_sprite.h>
#include <spice_graphics.h>

static sp_sprite_manager sprite_manager = {0};

void spiceSpriteInit(uint32_t sprite_count){
    sprite_manager._sprite_count = sprite_count;
    sprite_manager.sprites = (sp_sprite*)malloc(sizeof(sp_sprite) * sprite_count);
    memset(sprite_manager.sprites, 0, sizeof(sp_sprite) * sprite_count);
    sprite_manager._window_target = spiceGraphicsWindowTarget();
    atexit(spiceSpriteClose);
}

sp_sprite* spiceLoadSprite(char* path){
    uint64_t id = spHash(path, strlen(path));

    sp_sprite* found_sprite = NULL;
    for (size_t i = 0; i < sprite_manager._sprite_count; i++){
        if(sprite_manager.sprites[i]._id == id){ // Sprite already loaded, return it. Or a collision. Oop-
            sprite_manager.sprites[i]._ref_count++;
            return &sprite_manager.sprites[i];
        }
        
        if(sprite_manager.sprites[i]._ref_count == 0){
            found_sprite = &sprite_manager.sprites[i];
        }
    }

    // Out of sprite space
    if(found_sprite == NULL) return NULL;

    // Try to load the image...
    GPU_Image* img = GPU_LoadImage(path);
            
    if(img == NULL){
        return NULL; //and I oop- can't load the sprite
    }

    // We can load the sprite properly so now and only now do we finally remove this sprite image
    if(found_sprite->texture != NULL){
        GPU_FreeImage(found_sprite->texture);
        found_sprite->texture = NULL;
    }
    
    // Finalize setup and then return

    found_sprite->_ref_count++;
    found_sprite->texture = img;
    found_sprite->_id = id;
            
    return found_sprite;
}

void spiceFreeSprite(sp_sprite* sprite){
    if(sprite == NULL) return;
    sprite->_ref_count--;
}

void spiceDrawSprite(sp_sprite* sprite, float x, float y, float rotation, uint32_t row, float frame){
    if(sprite == NULL) return;
    GPU_Rect src = (GPU_Rect){((uint32_t)frame) * sprite->frame_w, row * sprite->frame_h, sprite->frame_w, sprite->frame_h};
    GPU_BlitTransform(sprite->texture, &src, sprite_manager._window_target, x, y, rotation, 1, 1);
}

void spiceSpriteClose(){
    for (size_t i = 0; i < sprite_manager._sprite_count; i++){
        if(sprite_manager.sprites[i].texture != NULL){
            GPU_FreeImage(sprite_manager.sprites[i].texture);
        }
    }
    free(sprite_manager.sprites);
}