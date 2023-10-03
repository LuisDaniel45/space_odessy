#include "states.h"

void game_over_state_load(global_t g)
{
    state_machine[cur_state].self = NULL;
}
    
void game_over_state_update(global_t g, double dt, char *KeyDown, int keypress)
{
    if (keypress == XK_r) 
    {
        sound_play(g.sounds, SOUND_SELECT);
        return change_state(g, STATE_PLAY);
    }
}

void game_over_state_render(global_t g)
{
    render_text(g.v_window, g.font, "Game Over", 
                (VW / 2) - (25*3) + (25/2), VH / 3, 25, 0x00ff0000);

    render_text(g.v_window, g.font, "Press R to Restart", 
                (VW / 2) - (10 * 5), VH / 2, 10,  0xffffffff);
}
