#include "states.h"

typedef font_t self_t;
void game_over_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    *(self_t*)state_machine[cur_state].self = font_init(xorg, "fixed");

}
    
void game_over_state_update(x11_t xorg, double dt, char *KeyDown, int keypress)
{
    if (keypress == XK_r) 
        return change_state(xorg);
}

void game_over_state_render(x11_t xorg)
{
    self_t *self  = state_machine[cur_state].self;
    print_screen(xorg, "Game Over", *self, 
                 xorg.v_window.w / 2, xorg.v_window.h / 3);
    print_screen(xorg, "Press R to Restart", *self, 
                 xorg.v_window.w / 2 - 10, xorg.v_window.h/ 2);
}
