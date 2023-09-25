#include "global.h"
#include "states.h"
#include <AL/al.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef font_t self_t;
void start_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    self_t *self = state_machine[cur_state].self;
}

void start_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    self_t *self = state_machine[cur_state].self;
    if (keypress == XK_Return)
    {
        sound_play(xorg.sounds, SOUND_SELECT);
        return change_state(xorg, STATE_PLAY);
    }
}

void start_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;

    xorg.font.w = 25;
    xorg.font.h = 25;
    render_text(xorg.v_window, xorg.font, "Space Odessy", 
                (VW / 2) - (xorg.font.w* 16) + (xorg.font.w/4), VH / 3, 0xffffffff);

    xorg.font.w = 10;
    xorg.font.h = 10;
    render_text(xorg.v_window, xorg.font, "Press Enter to Play", 
                (VW / 2) - (10 * 6), VH / 2, 0xffffffff);
}
