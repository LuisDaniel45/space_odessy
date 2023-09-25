#include "global.h"
#include "states.h"
#include <stdio.h>
#include <stdlib.h>

typedef font_t self_t;
static sound_t sound;
void game_over_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    self_t *self = state_machine[cur_state].self;
    if (font_init("font.ttf", self)) 
    {
        printf("Error loading font\n");
        exit(1);
    }
    sound = load_sound_file(xorg.sound, "select.wav");
}
    
void game_over_state_update(x11_t xorg, double dt, char *KeyDown, int keypress)
{
    self_t *self  = state_machine[cur_state].self;
    if (keypress == XK_r) 
    {
        free_font(*self);
        sound_play(sound);
        return change_state(xorg, STATE_PLAY);
    }
}

void game_over_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;

    self->w = 25;
    self->h = 25;
    render_text(xorg.v_window, *self, "Game Over", 
                (VW / 2) - (self->w * 3), VH / 3, 0x00ff0000);

    self->w = 10;
    self->h = 10;
    render_text(xorg.v_window, *self, "Press R to Restart", 
                (VW / 2) - (self->w * 5), VH / 2, 0xffffffff);
}
