#include "global.h"
#include "states.h"

typedef struct {
    font_t title;
    font_t text;
}self_t;

void game_over_state_load(x11_t xorg)
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
    
void game_over_state_update(x11_t xorg, double dt, char *KeyDown, int keypress)
{
    self_t *self  = state_machine[cur_state].self;
    if (keypress == XK_r) 
    {
        free_font(self->title);
        free_font(self->text);
        return change_state(xorg);
    }
}

void game_over_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;
    render_text(xorg.v_window, self->title, "Game Over", 
                (VW / 2) - (self->title.w * 8) + (self->title.w / 2), VH / 3, 0xffffffff);
    render_text(xorg.v_window, self->text, "Press R to Restart", 
                (VW / 2) - (self->text.w * 18), VH / 2, 0xffffffff);
    /** self_t *self  = state_machine[cur_state].self; */
    /** print_screen(xorg, "Game Over", *self,  */
    /**              xorg.v_window.w / 2, xorg.v_window.h / 3); */
    /** print_screen(xorg, "Press R to Restart", *self,  */
    /**              xorg.v_window.w / 2 - 10, xorg.v_window.h/ 2); */
}
