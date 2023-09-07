#include "global.h"
#include <stdio.h>

#define SPEED_PLAYER 400
#define GRAVITY 200
#define SHOT_SPEED 1000 
#define ASTEROIDS_SPEED 300 


// play state variables
static xcb_gcontext_t gc;
typedef struct node {
    xcb_rectangle_t pos;
    struct node *next;
} obj;

// player variables
static int player_color = 0x000000ff;
static xcb_rectangle_t player = {
    .x = 0,
    .y = 0,
    .width = 100,
    .height = 100
};

// shooting mechanic 
static obj *shots = NULL;
static int shot_color = 0x00ff0000;
void shoot();
void render_shots();
void update_shots(double);
void delete_shot();

// asteroids
static obj *asteroids = NULL;
static int asteroids_color = 0x00939597;
void spawn_asteroids();
void update_asteroids(double);
void render_asteroids();


char collision(xcb_rectangle_t a, xcb_rectangle_t b);

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

    if (rand() % 9000 == 500)
        spawn_asteroids();

    update_shots(dt);
    update_asteroids(dt);

        
    if (player.y < 0) 
        player.y = 0;
    else if (player.y + player.height > window.height)
        player.y = window.height - player.height;
    
    if (player.x < 0)
        player.x = 0;
    else if (player.x + player.width > window.width)
        player.x = window.width - player.width;
}

char shot_collision(xcb_rectangle_t object)
{
    for (obj *shot = shots; shot; shot = shot->next) 
        if (collision(shot->pos, object)) 
            return 1;
    return 0;    
}

void play_state_render()
{
    render_shots();
    render_asteroids();
    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &player_color);
    xcb_void_cookie_t cookie = xcb_poly_fill_rectangle(connection, 
                                                       window.id, 
                                                       gc, 1, 
                                                       &player); 
    xcb_request_check(connection, cookie);
}

char collision(xcb_rectangle_t a, xcb_rectangle_t b) 
{
    return (b.x < a.x + a.width &&
            b.x + b.width > a.x) && 
            (b.y < a.y + a.height && 
            b.y + b.height > a.y);
}

void spawn_asteroids()
{
    obj *asteroid = malloc(sizeof(obj));

    int size = (rand() % 100) + 50;
    asteroid->pos.height = size; 
    asteroid->pos.width = size;
    asteroid->pos.x = rand() % window.width;
    asteroid->pos.y = -asteroid->pos.height;
    asteroid->next = NULL;

    if (!asteroids) {
        asteroids = asteroid;
        return;
    }

    asteroid->next = asteroids; 
    asteroids = asteroid;
}

void update_asteroids(double dt) 
{
    if (!asteroids) 
        return;

    for (obj *asteroid = asteroids, *back = NULL; asteroid; asteroid = asteroid->next) 
    {
        if (asteroid->pos.y > window.height || shot_collision(asteroid->pos)) 
        {
            if (asteroids == asteroid) 
            {
                asteroids = asteroid->next;
                free(asteroid);
                return;
            } 
            back->next = asteroid->next;
            free(asteroid);
            asteroid = back->next;
            if (!asteroid)
                return;
        }

        double dy = ASTEROIDS_SPEED * dt;
        asteroid->pos.y += (int) dy;
        back = asteroid;
    }
}

void render_asteroids()
{
    if (!asteroids) 
        return;

    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &asteroids_color);
    for (obj *asteroid = asteroids; asteroid ; asteroid = asteroid->next) 
        xcb_poly_fill_rectangle(connection, 
                                window.id, 
                                gc, 1, 
                                &asteroid->pos); 
    xcb_flush(connection);
}

void shoot()
{
    obj *shot =  malloc(sizeof(obj));
    shot->pos.x = player.x + (player.width / 2);
    shot->pos.y = player.y;
    shot->pos.width = 5;
    shot->pos.height = 20;
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
    
    for (obj *shot = shots, *back = NULL; shot ; shot = shot->next) 
    {
        if (shot->pos.y < 0) 
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

        double dy = SHOT_SPEED * dt;
        shot->pos.y -= (int) dy; 
        back = shot;
    }
}

void render_shots()
{
    if (!shots) 
        return;

    xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, &shot_color);
    for (obj *shot = shots; shot != NULL; shot = shot->next) 
        xcb_poly_fill_rectangle(connection, 
                                window.id, 
                                gc, 1, 
                                &shot->pos); 
    xcb_flush(connection);
}
