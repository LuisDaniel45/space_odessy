#include "global.h"
#include "states.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef font_t self_t;
void start_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    self_t *self = state_machine[cur_state].self;
    font_init("font.ttf", self);
}

void start_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    self_t *self = state_machine[cur_state].self;
    if (keypress == XK_Return)
    {
        free_font(*self);
        return change_state(xorg, STATE_PLAY);
    }
}

void start_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;

    self->w = 25;
    self->h = 25;
    render_text(xorg.v_window, *self, "Space Odessy", 
                (VW / 2) - (self->w* 16) + (self->w/4), VH / 3, 0xffffffff);

    self->w = 10;
    self->h = 10;
    render_text(xorg.v_window, *self, "Press Enter to Play", 
                (VW / 2) - (20 * 18), VH / 2, 0xffffffff);
}
