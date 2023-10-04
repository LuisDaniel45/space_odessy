#include "states.h"

void start_state_load(global_t g)
{
    state_machine[cur_state].self = NULL;
}

void start_state_update(global_t g, double dt, int *KeyDown[], int keypress)
{
    if (keypress == KEY_enter)
    {
        sound_play(g.sounds, SOUND_SELECT);
        return change_state(g, STATE_PLAY);
    }
}

void start_state_render(global_t g)
{
    render_text(g.v_window, g.font, "Space Odessy",
                (VW / 2) - (25 * 4) + (25/2), VH / 3, 25, 0xffffffff);

    render_text(g.v_window, g.font, "Press Enter to Play",
                (VW / 2) - (10 * 6), VH / 2, 10, 0x0000ff00);
}
