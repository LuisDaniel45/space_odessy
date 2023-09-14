#include "states.h"
#include <stdlib.h>

typedef font_t self_t;
void start_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
    *((self_t*)state_machine[cur_state].self) = font_init(xorg, "fixed");
}

void start_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    if (keypress == XK_Return)
        return change_state(xorg);
}

void start_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self; 
    print_screen(xorg, "Space Odessy", *self, xorg.window.width / 2, xorg.window.height / 3);
    print_screen(xorg, "Press Enter to play", *self, (xorg.window.width / 2) - 20, xorg.window.height / 2);
}
