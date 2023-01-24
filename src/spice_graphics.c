#include <spice_graphics.h>
#include <glew.h>

static spice_graphics graphics = {0};

void spiceGraphicsInit(int width, int height, int target_fps, int window_flags){
    glewExperimental = GL_TRUE;
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        return; //fail
    }

    GPU_SetRequiredFeatures(GPU_FEATURE_BASIC_SHADERS);
    graphics.window_target = GPU_InitRenderer(GPU_RENDERER_OPENGL_3, width, height, window_flags);

    graphics.target_fps = target_fps;
    graphics.ticks_per_frame = 1000 / target_fps;
    graphics.cur_time = SDL_GetTicks64();

    graphics.clear_color = (SDL_Color){35, 35, 35, SDL_ALPHA_OPAQUE};

    graphics.camera = GPU_GetDefaultCamera();

    atexit(spiceGraphicsClose);
}

GPU_Target* spiceGraphicsWindowTarget(){
    return graphics.window_target;
}

void spiceGraphicsClose(){
    
    GPU_Quit();
    SDL_Quit();
}

void spiceGraphicsSetClearColor(sp_vec4 color){
    graphics.clear_color = (SDL_Color){(Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a};
}

void spiceGraphicsDraw(){
    GPU_SetCamera(graphics.window_target, &graphics.camera);

    GPU_Flip(graphics.window_target);
    GPU_ClearColor(graphics.window_target, graphics.clear_color);
}

void spiceGraphicsSetCamera(float x, float y, float zoom, float angle){
    graphics.camera.x = x;
    graphics.camera.y = y;
    graphics.camera.zoom_x = zoom;
    graphics.camera.zoom_y = zoom;
    graphics.camera.angle = angle;
}

void spiceGraphicsSetFullscreen(uint8_t fullscreen, uint8_t update_resolution){
    GPU_SetFullscreen(fullscreen, update_resolution);
}

void spiceGraphicsSetResolution(int width, int height){
    GPU_SetWindowResolution(width, height);
}

void spiceGraphicsScreenMod(sp_vec4 color){
    GPU_SetTargetColor(graphics.window_target, (SDL_Color){(Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a}); 
}

void spiceGraphicsStep(){
    graphics.prev_time = graphics.cur_time;
    graphics.cur_time = SDL_GetTicks64();

    uint64_t cur_frame_ticks = graphics.cur_time - graphics.prev_time;

    graphics.time += 1/graphics.target_fps;

    if(cur_frame_ticks < graphics.ticks_per_frame){
        SDL_Delay(graphics.ticks_per_frame - cur_frame_ticks);
    }

}