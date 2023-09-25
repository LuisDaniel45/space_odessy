#include "global.h"
#include "states.h" 
#include "objects/objects.h"

#include <AL/al.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>


#define PLAYER_SPEED 100 
#define GRAVITY 50 

typedef struct {
    xcb_gcontext_t gc;
    entity_t player;
    obj *asteroids;
    obj *shots;
    sound_t shoot_sound;
    sound_t game_over_sound;
    sound_t accelerate_sound;
} self_t;

#define PLAYER_ACCELERATE  1
#define PLAYER_TURN_LEFT  (1<<1)
#define PLAYER_TURN_RIGHT (1<<2)

#define PLAYER_MAX_SKINS 6
static image_t skins[PLAYER_MAX_SKINS];
void change_skins(int flags);

void play_state_load(x11_t xorg)
{
    load_asteroids(xorg);
    state_machine[cur_state].self = malloc(sizeof(self_t)); 
    self_t *self = state_machine[cur_state].self;

    self->shoot_sound = load_sound_file(xorg.sound, "shoot.wav");
    self->game_over_sound = load_sound_file(xorg.sound, "game_over.wav");
    self->accelerate_sound = load_sound_file(xorg.sound, "launch.wav");
    self->gc = xcb_generate_id(xorg.connection);
    xcb_create_gc(xorg.connection, self->gc, xorg.window.id, 0, NULL);

    skins[0] = slice_texture(xorg.textures, 0, 0, 50, 50, 0); 
    skins[1] = slice_texture(xorg.textures, 50, 0, 50, 50, 0);
    skins[2] = slice_texture(xorg.textures, 0, 50, 25, 50, 0);
    skins[3] = slice_texture(xorg.textures, 50, 50, 25, 50, 0);
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

void play_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    char flags = 0;
    self_t *self = state_machine[cur_state].self;
    double dx = 0, dy = 0;
    if (KeyDown[XK_j]) 
        dy = PLAYER_SPEED * dt;
    else if (KeyDown[XK_k]) {
        dy = -PLAYER_SPEED * dt;
        flags |= PLAYER_ACCELERATE;
    }
    if (KeyDown[XK_l]) {
        dx = PLAYER_SPEED * dt;
        flags |= PLAYER_TURN_RIGHT;
    }
    else if (KeyDown[XK_h]) {
        dx = -PLAYER_SPEED * dt;
        flags |= PLAYER_TURN_LEFT;
    }
    change_skins(flags);
    if (flags &= PLAYER_ACCELERATE) {
        int state;
        alGetSourcei(self->accelerate_sound.source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            sound_play(self->accelerate_sound);
        }
    }
    else {
        alSourcePause(self->accelerate_sound.source);
    }

    
    
    switch (keypress) {
        case 0:
            dy += GRAVITY * dt;
            break;

        case XK_space:
            shoot(&self->shots, self->player.pos);
            sound_play(self->shoot_sound);
            break;
    }
    self->player.pos.y = (int) round((double) self->player.pos.y  + dy);
    self->player.pos.x = (int) round((double) self->player.pos.x + dx);

    if (rand() % 100 == 0) 
        spawn_asteroids(xorg, &self->asteroids);

    update_shots(&self->shots, dt);

    if (update_asteroids(&self->asteroids, self->shots, self->player.pos, xorg, dt)) 
    {
        sound_play(self->game_over_sound); 
        unload_sound_file(self->accelerate_sound);
        unload_sound_file(self->shoot_sound);
        free_obj(self->shots);
        unload_asteroids(self->asteroids);
        xcb_free_gc(xorg.connection, self->gc);
        return change_state(xorg, STATE_GAME_OVER);
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


void play_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;

    render_image(xorg.v_window,
                  self->player.skin,
                  self->player.pos.x - self->player.x_offset,
                  self->player.pos.y - self->player.y_offset);
    render_asteroids(self->asteroids, xorg.v_window);
    render_shots(self->shots, xorg, self->gc);
}

char collision(xcb_rectangle_t a, xcb_rectangle_t b) 
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
