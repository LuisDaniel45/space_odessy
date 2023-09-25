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
    xorg.font.w = 25;
    xorg.font.h = 25;
    render_text(xorg.v_window, xorg.font, "Game Over", 
                (VW / 2) - (xorg.font.w * 3) + (xorg.font.w/2), VH / 3, 0x00ff0000);

    xorg.font.w = 10;
    xorg.font.h = 10;
    render_text(xorg.v_window, xorg.font, "Press R to Restart", 
                (VW / 2) - (xorg.font.w * 5), VH / 2, 0xffffffff);
}
