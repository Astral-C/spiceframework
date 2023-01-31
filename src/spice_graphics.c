#include <spice_graphics.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static spice_graphics graphics = {0};

void spiceGraphicsInit(char* window_name, int width, int height, int target_fps, int window_flags){
    //switch out for glad?
    
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        return; //fail
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    // TODO: LOTS OF ERROR CHECKING

    graphics.window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | window_flags);
    graphics.context = SDL_GL_CreateContext(graphics.window);
    SDL_GL_MakeCurrent(graphics.window, graphics.context);

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    graphics.target_fps = target_fps;
    graphics.ticks_per_frame = 1000 / target_fps;
    graphics.cur_time = SDL_GetTicks64();

    graphics.clear_color = (SDL_Color){35, 35, 35, SDL_ALPHA_OPAQUE};

    glViewport(0, 0, width, height);

    atexit(spiceGraphicsClose);
}

void spiceGraphicsClose(){
    
    SDL_GL_DeleteContext(graphics.context);
    SDL_DestroyWindow(graphics.window);

    SDL_Quit();
}

void spiceGraphicsSetClearColor(sp_vec4 color){
    glClearColor((Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a); 
}

void spiceGraphicsDraw(){
    SDL_GL_SwapWindow(graphics.window);
    //glClear(GL_COLOR_BUFFER_BIT);
}

void spiceGraphicsSetCamera(float x, float y, float zoom, float angle){

}

void spiceGraphicsSetFullscreen(uint8_t fullscreen, uint8_t update_resolution){

}

void spiceGraphicsSetResolution(int width, int height){
    SDL_SetWindowSize(graphics.window, width, height);
    glViewport(0, 0, width, height);
}

//this needs to be fixed for gl
// add render to texture _eventually_
void spiceGraphicsScreenMod(sp_vec4 color){
    glClearColor((Uint8)color.r, (Uint8)color.g, (Uint8)color.b, (Uint8)color.a); 
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