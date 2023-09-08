#include "global.h"

static font_t font;
void game_over_state_load()
{
    font = font_init("fixed");

}
    
void game_over_state_update(double dt, char *KeyDown, int keypress)
{
    if (keypress == XK_r) 
        change_state(STATE_START);
    
}

void game_over_state_render()
{
    print_screen("Game Over", font, window.width / 2, window.height / 3);
    print_screen("Press R to Restart", font, window.width / 2 - 10, window.height / 2);
}
