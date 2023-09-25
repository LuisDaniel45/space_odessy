#include "global.h"
#include "states.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    font_t font;
    sound_t select_sound;
} self_t; 
void start_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    self_t *self = state_machine[cur_state].self;
    font_init("font.ttf", &self->font);
    self->select_sound = load_sound_file(xorg.sound, "select.wav");
}

void start_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    self_t *self = state_machine[cur_state].self;
    if (keypress == XK_Return)
    {
        free_font(self->font);
        sound_play(self->select_sound);
        return change_state(xorg, STATE_PLAY);
    }
}

void start_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;

    self->font.w = 25;
    self->font.h = 25;
    render_text(xorg.v_window, self->font, "Space Odessy", 
                (VW / 2) - (self->font.w* 16) + (self->font.w/4), VH / 3, 0xffffffff);

    self->font.w = 10;
    self->font.h = 10;
    render_text(xorg.v_window, self->font, "Press Enter to Play", 
                (VW / 2) - (20 * 18), VH / 2, 0xffffffff);
}
