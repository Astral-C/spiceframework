#include <spice_graphics.h>

static spice_graphics graphics = {0};

void spiceGraphicsInit(int width, int height, int target_fps, int window_flags, char* post_processing_vertex, char* post_processing_fragment){
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        return; //fail
    }

    graphics.window_target = GPU_Init(width, height, window_flags);

    graphics.target_fps = target_fps;
    graphics.ticks_per_frame = 1000 / target_fps;
    graphics.cur_time = SDL_GetTicks64();

    graphics.camera = GPU_GetDefaultCamera();

    if(post_processing_vertex != NULL && post_processing_fragment != NULL){
        graphics._post_processing_target = GPU_CreateImage(width, height, GPU_FORMAT_RGBA);
        GPU_GetTarget(graphics._post_processing_target);

        Uint32 pp_vtx = GPU_LoadShader(GPU_VERTEX_SHADER, post_processing_vertex);

        if(!pp_vtx){
            spice_error("Failed to compile vertex shader: %s", GPU_GetShaderMessage());
            GPU_FreeImage(graphics._post_processing_target);
            graphics._post_processing_target = NULL;
        }

        Uint32 pp_frag = GPU_LoadShader(GPU_FRAGMENT_SHADER, post_processing_fragment);

        if(!pp_frag){
            spice_error("Failed to compile fragment shader: %s", GPU_GetShaderMessage());
            GPU_FreeImage(graphics._post_processing_target);
            graphics._post_processing_target = NULL;
        }

        graphics._post_processing_shader = GPU_LinkShaders(pp_vtx, pp_frag);

        if(!graphics._post_processing_shader){
            spice_error("Failed to link shaders: %s", GPU_GetShaderMessage());
            GPU_FreeImage(graphics._post_processing_target);
            graphics._post_processing_target = NULL;
        }

        graphics._post_processing_shader_params = GPU_LoadShaderBlock(graphics._post_processing_shader, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
    } else {
        graphics._post_processing_target = NULL;
    }

    atexit(spiceGraphicsClose);
}

GPU_Target* spiceGraphicsWindowTarget(){
    if(graphics._post_processing_target != NULL && graphics._post_processing_target->target != NULL){
        return graphics._post_processing_target->target;
    } else {
        return graphics.window_target;
    }
}

void spiceGraphicsClose(){
    if(graphics._post_processing_shader) GPU_FreeShaderProgram(graphics._post_processing_shader);
    if(graphics._post_processing_target) GPU_FreeImage(graphics._post_processing_target);
    
    GPU_Quit();
    SDL_Quit();
}

void spiceGraphicsDraw(){
    if(graphics._post_processing_target != NULL && graphics._post_processing_target.target){
        GPU_SetCamera(graphics._post_processing_target->target, &graphics.camera);
        GPU_ActivateShaderProgram(graphics._post_processing_shader, &graphics._post_processing_shader_params);
        GPU_SetUniformf(GPU_GetUniformLocation(graphics._post_processing_shader, "time"), graphics.time);
        GPU_Blit(graphics._post_processing_target, NULL, graphics.window_target, graphics.window_target->w / 2, graphics.window_target->h / 2);
        GPU_DeactivateShaderProgram();
    } else {
        GPU_SetCamera(graphics.window_target, &graphics.camera);
    }

    GPU_Flip(graphics.window_target);
    GPU_ClearColor(graphics.window_target, (SDL_Color){35, 35, 35, SDL_ALPHA_OPAQUE});
}

void spiceGraphicsSetCamera(float x, float y, float zoom, float angle){
    graphics.camera.x = x;
    graphics.camera.y = y;
    graphics.camera.zoom_x = zoom;
    graphics.camera.zoom_y = zoom;
    graphics.camera.angle = angle;
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

void spiceGraphicsDrawPolygon(sp_convex_polygon* polygon, SDL_Color color){
    GPU_Polygon(graphics.window_target, polygon->point_count, (float*)polygon->points, color);
}