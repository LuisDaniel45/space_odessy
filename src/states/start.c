#include "states.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_OPTIONS 2

typedef int self_t;
void start_state_load(global_t g)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
     *(self_t*)state_machine[cur_state].self = 0; 
}

void start_state_update(global_t *g, double dt, int *KeyDown[], int keypress)
{
    self_t* self = state_machine[cur_state].self; 
    switch (keypress) {
        case KEY_select:
            sound_play(&g->sounds[SOUND_SELECT]);
            if ((*self) == 0) 
                return change_state(*g, STATE_PLAY);
            else if ((*self) == 1)
                return change_state(*g, STATE_CONFIG);
            break;

        case KEY_down:
            sound_play(&g->sounds[SOUND_MOVE]);
            if (*self == MAX_OPTIONS - 1) 
                break;
            (*self)++;
            break;

        case KEY_up:
            sound_play(&g->sounds[SOUND_MOVE]);
            if (*self == 0) 
                break;
            (*self)--;
            break;
    
    }
}

void start_state_render(global_t g)
{
    self_t* self = state_machine[cur_state].self; 
    render_text(g.v_window, g.font, "Space Odessy",
                (int)(VW / 2) - (25 * 4) + (int)(25/2), (int)VH / 3, 25, 0xffffff00);

    render_text(g.v_window, g.font, "Play",
                (int)(VW / 2) - (10 ), VH / 2, 15, 
                (*self == 0)? 0x0000ff00: 0xffffffff);

    render_text(g.v_window, g.font, "settings",
                (int)(VW / 2) - (10 * 2), (int)(VH / 2) + 25, 15, 
                (*self == 1) ? 0x0000ff00: 0xffffffff);
}
