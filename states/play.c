#include "global.h"

#define SPEED_PLAYER 400
#define GRAVITY 200

typedef struct node {
    xcb_rectangle_t shot;
    struct node *next;
} shot_t;

static xcb_gcontext_t gc;
static int player_color = 0x000000ff;
static int shot_color = 0x00ff0000;
static shot_t *shots = NULL;
static xcb_rectangle_t player = {
    .x = 0,
    .y = 0,
    .width = 100,
    .height = 100
};

void shoot();
void render_shots();
void update_shots(double);
void delete_shot();

void play_state_load()
{
    gc = xcb_generate_id(connection);
    xcb_create_gc(connection, gc, window.id, 0, NULL);

    player.x = window.width / 2;
    player.y = window.height - player.height;
}

void play_state_update(double dt, char KeyDown[], int keypress)
{
    double dx = 0, dy = 0;
    if (KeyDown[XK_j]) 
        dy = SPEED_PLAYER * dt;
    if (KeyDown[XK_k]) 
        dy = -SPEED_PLAYER * dt;
    if (KeyDown[XK_l]) 
        dx = SPEED_PLAYER * dt;
    if (KeyDown[XK_h]) 
        dx = -SPEED_PLAYER * dt;
    
    switch (keypress) {
        case 0:
            dy += (GRAVITY * dt);
            break;

        case XK_space:
            shoot();
            break;
    }
    player.y += (int) dy;
    player.x += (int) dx;

    update_shots(dt);
        
    if (player.y < 0) 
        player.y = 0;
    else if (player.y + player.height > window.height)
        player.y = window.height - player.height;
    
    if (player.x < 0)
        player.x = 0;
    else if (player.x + player.width > window.width)
        player.x = window.width - player.width;
}

void play_state_render()
{
    render_shots();
    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &player_color);
    xcb_void_cookie_t cookie = xcb_poly_fill_rectangle(connection, 
                                                       window.id, 
                                                       gc, 1, 
                                                       &player); 
    xcb_request_check(connection, cookie);
}

void shoot()
{
    shot_t *shot =  malloc(sizeof(shot_t));
    shot->shot.x = player.x + (player.width / 2);
    shot->shot.y = player.y;
    shot->shot.width = 5;
    shot->shot.height = 20;
    shot->next = NULL;

    if (!shots) {
        shots = shot;
        return;
    }

    shot->next = shots;
    shots = shot;
}

void update_shots(double dt) 
{
    if (!shots) 
        return;
    
    for (shot_t *shot = shots, *back = NULL; shot ; shot = shot->next) 
    {
        if (shot->shot.y < 0) 
        {
            if (shots == shot) 
            {
                shots = shot->next;
                free(shot);
                return;
            } 
            back->next = shot->next; 
            free(shot);
            shot = back->next;
            if (!shot) 
                return;
        }

        shot->shot.y -= GRAVITY * dt; 
        back = shot;
    }
}

void render_shots()
{
    if (!shots) 
        return;

    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &shot_color);
    for (shot_t *shot = shots; shot != NULL; shot = shot->next) 
        xcb_poly_fill_rectangle(connection, 
                                window.id, 
                                gc, 1, 
                                &shot->shot); 
    xcb_flush(connection);
}
