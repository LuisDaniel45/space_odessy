#include "objects.h"

static xcb_image_t *skin;

void load_asteroids(x11_t xorg)
{
    skin = load_image("asteroids.png", 0, 0, xorg);
}

void spawn_asteroids(x11_t xorg, obj **asteroids, window_t window)
{
    obj *asteroid = malloc(sizeof(obj));

    int size = (rand() % 100) + 50;
    asteroid->entity.pos.height = size; 
    asteroid->entity.pos.width = size;
    asteroid->entity.pos.x = rand() % window.width;
    asteroid->entity.pos.y = -asteroid->entity.pos.height;
    asteroid->entity.skin = resize_image(skin, size + 100, size + 100);
    asteroid->entity.x_offset = 35;
    asteroid->entity.y_offset = 35;

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
        if (collision(asteroid->entity.pos, player)) 
            return 1;

        else if (asteroid->entity.pos.y > xorg.window.height || shot_collision(shots, asteroid->entity.pos))
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
        asteroid->entity.pos.y += (int) dy;
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
    {
        xcb_image_put(xorg.connection, 
                      xorg.window.pixmap, gc, 
                      asteroid->entity.skin, 
                      asteroid->entity.pos.x - asteroid->entity.x_offset, 
                      asteroid->entity.pos.y - asteroid->entity.y_offset, 0);

        /** xcb_poly_rectangle(xorg.connection,  */
        /**                    xorg.window.id, gc, 1,  */
        /**                    &asteroid->entity.pos); */
    }
    xcb_flush(xorg.connection);
}
