#include "objects.h"

char shot_collision(obj *shots, rectangle_t object)
{
    for (obj *shot = shots; shot; shot = shot->next) {
        if (collision(shot->entity.pos, object)) {
            shot->entity.pos.y = -shot->entity.pos.height;
            return 1;
        }
    }
    return 0;    
}

void shoot(obj **shots, rectangle_t player)
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

void render_shots(obj *shots, global_t g, color_t color)
{
    if (!shots) 
        return;

    for (obj *shot = shots; shot != NULL; shot = shot->next) 
         render_rectangle(g, shot->entity.pos, color);
}
