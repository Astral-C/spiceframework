#include <spice_graphics.h>

static spice_graphics graphics = {0};

void spiceGraphicsInit(int width, int height, int target_fps, int window_flags){
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        return; //fail
    }

    graphics.window_target = GPU_Init(width, height, GPU_DEFAULT_INIT_FLAGS);

    graphics.target_fps = target_fps;
    graphics.ticks_per_frame = 1000 / target_fps;
    graphics.cur_time = SDL_GetTicks64();

    atexit(spiceGraphicsClose);
}

GPU_Target* spiceGraphicsWindowTarget(){
    return graphics.window_target;
}

void spiceGraphicsClose(){
    GPU_Quit();
    SDL_Quit();
}

void spiceGraphicsDraw(){
    GPU_Flip(graphics.window_target);
    GPU_ClearColor(graphics.window_target, (SDL_Color){35, 35, 35, SDL_ALPHA_OPAQUE});
}

void spiceGraphicsStep(){
    graphics.prev_time = graphics.cur_time;
    graphics.cur_time = SDL_GetTicks64();

    uint64_t cur_frame_ticks = graphics.cur_time - graphics.prev_time;

    if(cur_frame_ticks < graphics.ticks_per_frame){
        SDL_Delay(graphics.ticks_per_frame - cur_frame_ticks);
    }

}