#pragma once
#include "global.h"

typedef enum {
    STATE_START,
    STATE_PLAY,
    STATE_GAME_OVER 
} states_id;

#include "start.c"
#include "play.c"
#include "game_over.c"

typedef struct { 
    states_id id;
    void (*load)();
    void (*render)();
    void (*update)(double, char[], int);
} state_t;

static const int number_of_states = STATE_GAME_OVER + 1;
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
    },
    {
        .id = STATE_GAME_OVER, 
        .load = game_over_state_load, 
        .render = game_over_state_render, 
        .update = game_over_state_update
    }
};
static states_id cur_state = STATE_START;


