#ifndef __INPUT_H__
#define __INPUT_H__
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
} spice_input_cmd;

typedef struct {
    uint32_t command_count;
    spice_input_cmd* commands;
} spice_input;

void spiceInputInit(char* input_file_path);
void spiceInputUpdate();
spice_input_cmd* spiceInputGetCmd(char* name);

// Get Simple Input State of key
uint8_t spiceSInputState(SDL_Scancode key);

#endif