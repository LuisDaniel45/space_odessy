#include "objects.h"

void spawn_asteroids(obj **asteroids, window_t window)
{
    obj *asteroid = malloc(sizeof(obj));

    int size = (rand() % 100) + 50;
    asteroid->pos.height = size; 
    asteroid->pos.width = size;
    asteroid->pos.x = rand() % window.width;
    asteroid->pos.y = -asteroid->pos.height;
    asteroid->next = NULL;

    if (!*asteroids) {
        *asteroids = asteroid;
        return;
    }

    asteroid->next = *asteroids; 
    *asteroids = asteroid;
}

char update_asteroids(obj **asteroids, obj *shots, xcb_rectangle_t player, x11_t xorg, double dt) 
{
    if (!*asteroids) 
        return 0;

    for (obj *asteroid = *asteroids, *back = NULL; asteroid; asteroid = asteroid->next) 
    {
        if (collision(asteroid->pos, player)) 
            return 1;

        else if (asteroid->pos.y > xorg.window.height || shot_collision(shots, asteroid->pos))
        {
            if (*asteroids == asteroid) 
            {
                *asteroids = asteroid->next;
                free(asteroid);
                return 0;
            } 
            back->next = asteroid->next;
            free(asteroid);
            asteroid = back->next;
            if (!asteroid)
                return 0;
        }

        double dy = ASTEROIDS_SPEED * dt;
        asteroid->pos.y += (int) dy;
        back = asteroid;
    }
    return 0;
}

void render_asteroids(obj *asteroids, xcb_gcontext_t gc, x11_t xorg)
{
    if (!asteroids) 
        return;

    int asteroids_color = ASTEROIDS_COLOR;
    xcb_change_gc(xorg.connection, gc, XCB_GC_FOREGROUND, &asteroids_color);
    for (obj *asteroid = asteroids; asteroid ; asteroid = asteroid->next) 
        xcb_poly_fill_rectangle(xorg.connection, 
                                xorg.window.id, 
                                gc, 1, 
                                &asteroid->pos); 
    xcb_flush(xorg.connection);
}
