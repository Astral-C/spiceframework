#include <spice_util.h>
#include <spice_gamestate.h>

static sp_gamestate_manager state_manager = {0};

void spiceGamestateClose(){
    if(state_manager.game_states != NULL){
        free(state_manager.game_states);
    }
}

void spiceGamestateInit(int state_count){
    state_manager.state_count = state_count + 2;
    state_manager.game_states = malloc(sizeof(sp_gamestate)*(state_count + 2));
    state_manager.game_states[SP_GAMESTATE_NONE] = (sp_gamestate){SP_GAMESTATE_NONE, -1, NULL, NULL};
    state_manager.game_states[SP_GAMESTATE_TRANSITION] = (sp_gamestate){SP_GAMESTATE_TRANSITION, -1, spiceGamestateTransitionUpdate, NULL};
    state_manager.current = &state_manager.game_states[0];
    state_manager.target = state_manager.current;
    state_manager.previous = state_manager.current;
    
    atexit(spiceGamestateClose);
}

void spiceGamestateChange(int id, int transition){
    if(SP_GAMESTATE_USER + id >= state_manager.state_count) return;
    if(transition == 0){
        state_manager.previous = state_manager.current;
        state_manager.current = &state_manager.game_states[SP_GAMESTATE_USER + id];
    } else {
        state_manager.current_transition_time = 0;
        state_manager.previous = state_manager.current;
        state_manager.target = &state_manager.game_states[SP_GAMESTATE_USER + id];
        state_manager.current = &state_manager.game_states[SP_GAMESTATE_TRANSITION];
    }

}

void spiceGamestateUpdate(){
    if(state_manager.current != NULL){
        if(state_manager.current->update != NULL) state_manager.current->update();
        if(state_manager.current->draw != NULL) state_manager.current->draw();
    }
}

int spiceGamestateGet(){
    return state_manager.current->id;
}

void spiceGamestateRegister(int id, void (*update)(), void (*draw)()){
    if(SP_GAMESTATE_USER + id >= state_manager.state_count) return;
    state_manager.game_states[SP_GAMESTATE_USER + id] = (sp_gamestate){SP_GAMESTATE_USER, id, update, draw};
    spice_info("Registered gamestate to %d with id %d\n", SP_GAMESTATE_USER + id, id);
}

void spiceGamestateSetTransitionTime(uint64_t time){
    state_manager.transition_time = time;
}

void spiceGamestateTransitionUpdate(){
    state_manager.current_transition_time += state_manager.transition_speed;
    if(state_manager.transition_func != NULL) state_manager.transition_func(state_manager.current_transition_time, state_manager.transition_time, state_manager.previous, state_manager.target);

    if(state_manager.current_transition_time >= state_manager.transition_time){
        state_manager.current = state_manager.target;
        state_manager.target = NULL;
    }
}

void spiceGamestateSetTransitionSpeed(uint64_t speed){
    state_manager.transition_speed = speed;
}

void spiceGamestateSetTransitionFunc(void (*user_transition_func)(uint64_t cur_time, uint64_t total_time, sp_gamestate* previous, sp_gamestate* next)){
    state_manager.transition_func = user_transition_func;
}