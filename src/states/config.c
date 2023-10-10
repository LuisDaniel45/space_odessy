#include "states.h"
#include "../global.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *template_selecet_string = "press key for '%s':";
struct options_t {
    char *text;
    int id;
};
struct options_t remapable_keys[] = {
    {.text = "quit", .id = KEY_quit },
    {.text = "move up", .id = KEY_up},
    {.text = "move down", .id = KEY_down},
    {.text = "move left", .id = KEY_left},
    {.text = "move right", .id = KEY_right},
    {.text = "shoot", .id = KEY_shoot},
};
#define MAX_OPTIONS (sizeof(remapable_keys) / sizeof(remapable_keys[0]))

typedef struct {
    char select_bool;
    int select;
    char *select_string;
    int select_string_len;
} self_t;

void config_state_load(global_t g)
{
    state_machine[cur_state].self = malloc(sizeof(self_t));
     ((self_t*)state_machine[cur_state].self)->select = 0; 
     ((self_t*)state_machine[cur_state].self)->select_string = malloc(255); 
     ((self_t*)state_machine[cur_state].self)->select_bool = 0; 
}

void config_state_update(global_t* g, double dt, int*key_down[], int keypress)
{
    self_t* self = state_machine[cur_state].self; 
    if (self->select_bool && **key_down) 
    {
        key_down[remapable_keys[self->select].id] = (*key_down + **key_down);
        self->select_bool = 0;
        return;
    }


    switch (keypress) {
        case KEY_select:
            sound_play(&g->sounds[SOUND_SELECT]);
            self->select_bool = 1; 
            sprintf(self->select_string, template_selecet_string, remapable_keys[self->select].text);
            self->select_string_len = strlen(self->select_string);
            break;

        case KEY_down:
            sound_play(&g->sounds[SOUND_MOVE]);
            if (self->select == MAX_OPTIONS - 1) 
                break;
            self->select++;
            break;

        case KEY_up:
            sound_play(&g->sounds[SOUND_MOVE]);
            if (self->select == 0) 
                break;
            self->select--;
            break;

        case KEY_b:
            sound_play(&g->sounds[SOUND_SELECT]);
            return change_state(*g, STATE_START);

    
    }
}

void config_state_render(global_t g)
{
    self_t* self = state_machine[cur_state].self; 
    render_text(g.v_window, g.font, "Settings:", 
            (VW / 2) - (10 * 10), VH / 4, 25, 0xffffff00);

    if (self->select_bool) 
    {
        render_text(g.v_window, g.font, self->select_string,
                (VW / 2) - (10 * ((int)self->select_string_len/2)), 
                VH / 3, 15, 0xffffffff);
        return;
    }
    
    for (int i = 0; i < MAX_OPTIONS; i++) 
        render_text(g.v_window, g.font, remapable_keys[i].text,
                (int)(VW / 2) - (10 * 2), (int)(VH / 3 + (25 * i)), 15, 
                (self->select == i)? 0x0000ff00: 0xffffffff);
}
