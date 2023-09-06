#include "global.h"
#include <stdio.h>

font_t font;

void start_state_load()
{
    font = font_init("fixed");
}

void start_state_update(double dt, int keypress)
{
    if (keypress == XK_Return)
        change_state();
}

void start_state_render()
{
    print_screen("Space Odessy", font, window.width / 2, window.height / 3);
    print_screen("Press Enter to play", font, (window.width / 2) - 20, window.height / 2);
}
