#include "states.h"
#include <stdio.h>
#include <stdlib.h>

static char score[255]; 
void game_over_state_load(global_t g)
{
    state_machine[cur_state].self = NULL;
    sprintf(score, "Score: %i", g.score);
    g.score = 0;
}
    
void game_over_state_update(global_t *g, double dt, int  *KeyDown[], int keypress)
{
    if (keypress == KEY_retry) 
    {
        sound_play(&g->sounds[SOUND_SELECT]);
        return change_state(*g, STATE_PLAY);
    }
}

void game_over_state_render(global_t g)
{
    render_text(g.v_window, g.font, "Game Over", 
                (VW / 2) - (25*3) + (25/2), VH / 3, 25, 0x00ff0000);

    render_text(g.v_window, g.font, score, 
            (VW / 2) - (10 * 2), (VH / 2.3), 15,  0xffffffff);

    render_text(g.v_window, g.font, "Press R to Restart", 
                (VW / 2) - (10 * 4) - 5, VH / 1.9, 10,  0xffffffff);

}
