#include "objects.h"
#include <stdio.h>
#include <stdlib.h>

char shot_collision(obj *shots, xcb_rectangle_t object)
{
    for (obj *shot = shots; shot; shot = shot->next) 
        if (collision(shot->pos, object)) {
            shot->pos.y = -shot->pos.height;
            return 1;
        }
    return 0;    
}

void shoot(obj **shots, xcb_rectangle_t player)
{
    obj *shot =  malloc(sizeof(obj));
    shot->pos.x = player.x + (player.width / 2);
    shot->pos.y = player.y;
    shot->pos.width = 5;
    shot->pos.height = 20;
    shot->next = NULL;

    if (!*shots) {
        *shots = shot;
        return;
    }

    shot->next = *shots;
    *shots = shot;
}

void update_shots(obj **shots, double dt) 
{
    if (!*shots) 
        return;
    
    for (obj *shot = *shots, *back = NULL; shot ; shot = shot->next) 
    {
        if (shot->pos.y < 0) 
        {
            if (*shots == shot) 
            {
                *shots = shot->next;
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

void render_shots(obj *shots, xcb_gcontext_t gc, x11_t xorg)
{
    if (!shots) 
        return;

    int shot_color = SHOT_COLOR;
    xcb_change_gc(xorg.connection, gc, XCB_GC_FOREGROUND, &shot_color);
    for (obj *shot = shots; shot != NULL; shot = shot->next) 
        xcb_poly_fill_rectangle(xorg.connection, 
                                xorg.window.id, 
                                gc, 1, 
                                &shot->pos); 
    xcb_flush(xorg.connection);
}
