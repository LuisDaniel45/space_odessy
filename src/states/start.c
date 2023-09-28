#include "states.h"

void start_state_load(x11_t xorg)
{
    state_machine[cur_state].self = NULL;
}

void start_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    if (keypress == XK_Return)
    {
        sound_play(xorg.sounds, SOUND_SELECT);
        return change_state(xorg, STATE_PLAY);
    }
}

void start_state_render(x11_t xorg)
{
    render_text(xorg.v_window, xorg.font, "Space Odessy",
                (VW / 2) - (25 * 4) + (25/2), VH / 3, 25, 0xffffffff);

    render_text(xorg.v_window, xorg.font, "Press Enter to Play",
                (VW / 2) - (10 * 6), VH / 2, 10, 0x0000ff00);
}
