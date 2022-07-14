#include <spice_input.h>

static const uint8_t* key_states = NULL;

static spice_input cmd_handler = {0};

void spiceInputClose(){
    if(cmd_handler.commands != NULL) free(cmd_handler.commands);
}

void spiceInputInit(char* input_file_path){
    FILE* input_cfg = fopen(input_file_path, "r");

    if(input_cfg == NULL){
        spice_error("Couldnt open file %s\n", input_file_path);
        return;
    }

    while(!feof(input_cfg)){
        if(getc(input_cfg) == '\n') cmd_handler.command_count++;
    }   

    cmd_handler.commands = malloc(sizeof(spice_input_cmd) * cmd_handler.command_count);
    memset(cmd_handler.commands, 0, sizeof(spice_input_cmd) * cmd_handler.command_count);

    rewind(input_cfg);

    int cmd_ptr = 0;
    for(size_t cmd_ptr = 0; cmd_ptr < cmd_handler.command_count; cmd_ptr++){
        if(feof(input_cfg)) break;

        sp_str command;
        char* cmd_name, *scancode;

        fscanf(input_cfg, "%s\n", command);
        cmd_name = strtok(command, ":");
        scancode = strtok(NULL, ":");

        sp_keycpy(cmd_handler.commands[cmd_ptr].command_name, cmd_name);
        cmd_handler.commands[cmd_ptr].key = SDL_GetScancodeFromName(scancode);
        if(cmd_handler.commands[cmd_ptr].key == SDL_SCANCODE_UNKNOWN){
            spice_error("Unknown scancode for cmd %s %d:%s\n", cmd_name, cmd_handler.commands[cmd_ptr].key, scancode);
        }
    }

    fclose(input_cfg);

    atexit(spiceInputClose);
}

spice_input_cmd* spiceInputGetCmd(char* name){
    for (spice_input_cmd* input = cmd_handler.commands; input != cmd_handler.commands + cmd_handler.command_count; input++){
        if(sp_keycmp(name, input->command_name)){
            return input;
        }
    }
}

void spiceInputUpdate(){
    key_states = SDL_GetKeyboardState(NULL);

    for (spice_input_cmd* input = cmd_handler.commands; input != cmd_handler.commands + cmd_handler.command_count; input++){
        if(key_states[input->key]){
            if(input->state == SP_INPUT_PRESSED) {
                input->state = SP_INPUT_HELD;
            } else if(input->state == SP_INPUT_NONE || input->state == SP_INPUT_RELEASED){
                input->state = SP_INPUT_PRESSED;
            }
        } else {
            if(input->state == SP_INPUT_HELD || input->state == SP_INPUT_PRESSED){
                input->state = SP_INPUT_RELEASED;
            } else {
                input->state = SP_INPUT_NONE;
            }
        }
    }
    
}

uint8_t spiceSInputState(SDL_Scancode key){ //simple input  checker
    return key_states[key];
}
