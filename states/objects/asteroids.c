#include "objects.h"
#include <stdlib.h>
#include <xcb/xproto.h>

static xcb_image_t *skin[MAX_SKINS];

void load_asteroids(x11_t xorg)
{
    skin[0] = load_image("asteroids.png", 0, 0, xorg);
    for (int i = 1; i < MAX_SKINS; i++) 
        skin[i] = resize_image(skin[0], skin[i - 1]->width + 10, skin[i - 1]->height + 10);
}

void spawn_asteroids(x11_t xorg, obj **asteroids, window_t window)
{
    obj *asteroid = malloc(sizeof(obj));

    xcb_image_t *ptr_skin  = skin[rand() % MAX_SKINS]; 
    double pw = (ptr_skin->height * 0.30);
    double ph = (ptr_skin->width * 0.30);  
    asteroid->entity.pos.height = (int) ptr_skin->height - ph;
    asteroid->entity.pos.width = (int) ptr_skin->width - pw;
    asteroid->entity.pos.x = rand() % (VW - asteroid->entity.pos.width);
    asteroid->entity.pos.y = -asteroid->entity.pos.height;
    asteroid->entity.skin = ptr_skin; 
    asteroid->entity.x_offset = (int)pw - (pw / 4);
    asteroid->entity.y_offset = (int)ph - (ph / 4); 

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

        else if (asteroid->entity.pos.y > VH || shot_collision(shots, asteroid->entity.pos))
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

    for (obj *asteroid = asteroids; asteroid ; asteroid = asteroid->next) {
        render_image(xorg.window.buffer,
                     asteroid->entity.skin,
                     asteroid->entity.pos.x - asteroid->entity.x_offset,
                     asteroid->entity.pos.y - asteroid->entity.y_offset);
        /** render_rectangle(xorg.window.buffer, asteroid->entity.pos, 0x0000ff00); */
    }
}
