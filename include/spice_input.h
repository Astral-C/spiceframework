#ifndef __SPICE_INPUT_H__
#define __SPICE_INPUT_H__
#include <SDL2/SDL.h>
#include <spice_util.h>

#define SP_INPUT_NONE     0x000000000
#define SP_INPUT_PRESSED  0x000000001
#define SP_INPUT_HELD     0x000000002
#define SP_INPUT_RELEASED 0x000000004

typedef struct {
    uint32_t state;
    SDL_KeyCode key;
    sp_key command_name;
} spice_input;

typedef struct {
    uint32_t command_count;
    spice_input* commands;
} spice_input_manager;

void spiceInputInit(char* input_file_path);
void spiceInputUpdate();
spice_input* spiceInputGetCmd(char* name);

// Get Simple Input State of key
uint8_t spiceSInputState(SDL_Scancode key);

#endif