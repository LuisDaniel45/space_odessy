#include "objects.h"

char shot_collision(obj *shots, xcb_rectangle_t object)
{
    for (obj *shot = shots; shot; shot = shot->next) {
        if (collision(shot->entity.pos, object)) {
            shot->entity.pos.y = -shot->entity.pos.height;
            return 1;
        }
    }
    return 0;    
}

void shoot(obj **shots, xcb_rectangle_t player)
{
    obj *shot =  malloc(sizeof(obj));
    shot->entity.pos.x = player.x + (player.width / 2);
    shot->entity.pos.y = player.y;
    shot->entity.pos.width = 2;
    shot->entity.pos.height = 10;
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
        if (shot->entity.pos.y < 0) 
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
        shot->entity.pos.y -= (int) dy; 
        back = shot;
    }
}

void render_shots(obj *shots, x11_t xorg, xcb_gcontext_t gc)
{
    if (!shots) 
        return;

    xcb_change_gc(xorg.connection, gc, XCB_GC_FOREGROUND, (int[]) {SHOT_COLOR});
    for (obj *shot = shots; shot != NULL; shot = shot->next) {
        xcb_rectangle_t scale_pos = translate_rect_pos(xorg.v_window, shot->entity.pos);
        xcb_poly_fill_rectangle(xorg.connection, xorg.v_window.pix, gc, 1, &scale_pos);
                                
    }
}
