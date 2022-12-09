#ifndef __SPICE_GAMESTATE_H__
#define __SPICE_GAMESTATE_H__
#include <stdint.h>

typedef enum {
    SP_GAMESTATE_NONE,
    SP_GAMESTATE_TRANSITION,
    SP_GAMESTATE_USER
} sp_gamestate_type;

typedef struct GS_S {
    sp_gamestate_type type;
    void (*update)();
    void (*draw)();
} sp_gamestate;

typedef struct GMS_S {
    sp_gamestate* previous;
    sp_gamestate* current;
    sp_gamestate* target;

    uint64_t current_transition_time;
    uint64_t transition_time;
    uint64_t transition_speed;
    
    void (*transition_func)(uint64_t cur_time, uint64_t total_time, sp_gamestate* previous, sp_gamestate* next);

    uint32_t state_count;
    sp_gamestate* game_states;
} sp_gamestate_manager;


void spiceGamestateInit(int state_count);

void spiceGamestateRegister(int id, void (*update)(), void (*draw)());
void spiceGamestateChange(int id, int transition);
void spiceGamestateUpdate();

void spiceGamestateSetTransitionTime(uint64_t time);
void spiceGamestateTransitionUpdate();
void spiceGamestateSetTransitionSpeed(uint64_t speed);
void spiceGamestateSetTransitionFunc(void (*user_transition_func)(uint64_t cur_time, uint64_t total_time, sp_gamestate* previous, sp_gamestate* next));

#endif