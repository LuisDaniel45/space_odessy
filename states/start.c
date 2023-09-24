#include "global.h"
#include "states.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    font_t title;
    font_t text;
}self_t;

void start_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    self_t *self = state_machine[cur_state].self;
    self->title.w = 50;
    self->title.h = 50;
    self->text.w = 20;
    self->text.h = 20;
    font_init("font.ttf", &self->title);
    font_init("font.ttf", &self->text);
}

void start_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    self_t *self = state_machine[cur_state].self;
    if (keypress == XK_Return)
    {
        free_font(self->title);
        free_font(self->text);
        return change_state(xorg);
    }
}

void start_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;
    render_text(xorg.v_window, self->title, "Space Odessy", 
                (VW / 2) - (self->title.w * 14) - (self->title.w/5), VH / 3, 0xffffffff);
    render_text(xorg.v_window, self->text, "Press Enter to Play", 
                (VW / 2) - (self->text.w * 18), VH / 2, 0xffffffff);
}
