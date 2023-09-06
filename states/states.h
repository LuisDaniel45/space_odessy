#pragma once
#include "global.h"
#include "start.c"
#include "play.c"

typedef enum {
    STATE_START,
    STATE_PLAY,
    END_STATE
} states_id;

typedef struct { 
    states_id id;
    void (*load)();
    void (*render)();
    void (*update)(double, char[], int);
} state_t;

static const int number_of_states = END_STATE + 1;
static state_t state_machine[] = {
    {
        .id = STATE_START, 
        .load = start_state_load, 
        .render = start_state_render, 
        .update = start_state_update
    },
    {
        .id = STATE_PLAY, 
        .load = play_state_load, 
        .render = play_state_render, 
        .update = play_state_update
    }
};
static states_id cur_state = STATE_START;


