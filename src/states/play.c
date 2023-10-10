#include "states.h" 
#include "../objects/objects.h"
#include <math.h>
#include <stdio.h>

#define PLAYER_SPEED 100 
#define GRAVITY 50 

typedef struct {
    entity_t player;
    obj *asteroids;
    obj *shots;
    color_t shots_color;
} self_t;

#define PLAYER_ACCELERATE  1
#define PLAYER_TURN_LEFT  (1<<1)
#define PLAYER_TURN_RIGHT (1<<2)

#define PLAYER_MAX_SKINS 6
static image_t skins[PLAYER_MAX_SKINS];
void change_skins(int flags);

void play_state_load(global_t g)
{
    load_asteroids(g);
    state_machine[cur_state].self = malloc(sizeof(self_t)); 
    self_t *self = state_machine[cur_state].self;

    self->shots_color = create_color(0x00ff0000, &g);

    skins[0] = slice_texture(g.textures, 0, 0, 50, 50, 0); 
    skins[1] = slice_texture(g.textures, 50, 0, 50, 50, 0);
    skins[2] = slice_texture(g.textures, 0, 50, 25, 50, 0);
    skins[3] = slice_texture(g.textures, 50, 50, 25, 50, 0);
    skins[4] = flip_image(skins[2]); 
    skins[5] = flip_image(skins[3]); 

    change_skins(0);
    self->player.pos.x = VW / 2;
    self->player.pos.y = VH - self->player.pos.height;
    self->player.x_offset = 10;
    self->player.y_offset = 10;

    self->shots = NULL;
    self->asteroids = NULL;
}

void play_state_update(global_t *g, double dt, int *KeyDown[], int keypress)
{
    char flags = 0;
    self_t *self = state_machine[cur_state].self;
    double dx = 0, dy = 0;
    if (is_key_down(KeyDown, KEY_down))
        dy = PLAYER_SPEED * dt;
    else if (is_key_down(KeyDown, KEY_up)) {
        dy = -PLAYER_SPEED * dt;
        flags |= PLAYER_ACCELERATE;
    }
    if (is_key_down(KeyDown, KEY_right)) {
        dx = PLAYER_SPEED * dt;
        flags |= PLAYER_TURN_RIGHT;
    }
    else if (is_key_down(KeyDown, KEY_left)) {
        dx = -PLAYER_SPEED * dt;
        flags |= PLAYER_TURN_LEFT;
    }
    change_skins(flags);
    if (flags &= PLAYER_ACCELERATE)
    {
        if (!sound_is_alive(g->sounds[SOUND_LAUNCH]))
            sound_play(&g->sounds[SOUND_LAUNCH]);
    }
    else
        sound_kill(g->sounds[SOUND_LAUNCH]);
    
    
    switch (keypress) {
        case 0:
            dy += GRAVITY * dt;
            break;

        case KEY_shoot:
            shoot(&self->shots, self->player.pos);
            sound_play(&g->sounds[SOUND_SHOOT]);
            break;
    }
    self->player.pos.y = (int) round((double) self->player.pos.y  + dy);
    self->player.pos.x = (int) round((double) self->player.pos.x + dx);

    if (rand() % 100 == 0) 
        spawn_asteroids(*g, &self->asteroids);

    update_shots(&self->shots, dt);

    if (update_asteroids(&self->asteroids, self->shots, self->player.pos, g, dt)) 
    {
        sound_kill(g->sounds[SOUND_LAUNCH]);
        sound_play(&g->sounds[SOUND_GAME_OVER]); 
        free_obj(self->shots);
        unload_asteroids(self->asteroids);
        color_free(self->shots_color, &g);
        return change_state(*g, STATE_GAME_OVER);
    }
        
    if (self->player.pos.y < 0) 
        self->player.pos.y = 0;
    else if (self->player.pos.y + self->player.pos.height > VH)
        self->player.pos.y = VH - self->player.pos.height;
    
    if (self->player.pos.x < 0)
        self->player.pos.x = 0;
    else if (self->player.pos.x + self->player.pos.width > VW)
        self->player.pos.x = VW - self->player.pos.width;
}


void play_state_render(global_t g)
{
    self_t *self = state_machine[cur_state].self;

    render_image(g.v_window,
                  self->player.skin,
                  self->player.pos.x - self->player.x_offset,
                  self->player.pos.y - self->player.y_offset);
    render_asteroids(self->asteroids, g.v_window);
    render_shots(self->shots, g, self->shots_color);
}

char collision(rectangle_t a, rectangle_t b) 
{
    return (b.x < a.x + a.width &&
            b.x + b.width > a.x) && 
            (b.y < a.y + a.height && 
            b.y + b.height > a.y);
}


void free_obj(obj *objects) 
{
    if (!objects) 
        return;

    while (objects) {
        obj *tmp = objects->next;
        free(objects);
        objects = tmp;
    }
}


void change_skins(int flags)
{
    self_t *self = state_machine[cur_state].self;
    self->player.skin = skins[flags]; 
    self->player.pos.width = skins[flags].width - (skins[flags].width / 2);
    self->player.pos.height = skins[flags].height - (skins[flags].height / 4);
}
