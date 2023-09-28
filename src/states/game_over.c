#include "states.h"

void game_over_state_load(x11_t xorg)
{
    state_machine[cur_state].self = NULL;
}
    
void game_over_state_update(x11_t xorg, double dt, char *KeyDown, int keypress)
{
    if (keypress == XK_r) 
    {
        sound_play(xorg.sounds, SOUND_SELECT);
        return change_state(xorg, STATE_PLAY);
    }
}

void game_over_state_render(x11_t xorg)
{
    render_text(xorg.v_window, xorg.font, "Game Over", 
                (VW / 2) - (25*3) + (25/2), VH / 3, 25, 0x00ff0000);

    render_text(xorg.v_window, xorg.font, "Press R to Restart", 
                (VW / 2) - (10 * 5), VH / 2, 10,  0xffffffff);
}
