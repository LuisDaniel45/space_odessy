#include "states.h" 
#include "objects/objects.h"

#define PLAYER_SPEED 400
#define GRAVITY 200
#define PLAYER_COLOR 0x000000ff

typedef struct {
    xcb_gcontext_t gc;
    xcb_rectangle_t player;
    obj *asteroids;
    obj *shots;
} self_t;

void free_obj(obj*);

void play_state_load(x11_t xorg)
{
    state_machine[cur_state].self = malloc(sizeof(self_t)); 
    self_t *self = state_machine[cur_state].self;
    self->gc = xcb_generate_id(xorg.connection);
    xcb_create_gc(xorg.connection, self->gc, xorg.window.id, 0, NULL);

    self->player.x = xorg.window.width / 2;
    self->player.y = xorg.window.height - self->player.height;
    self->player.width = 100;
    self->player.height= 100;
    self->shots = NULL;
    self->asteroids = NULL;
}

void play_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    self_t *self = state_machine[cur_state].self;
    double dx = 0, dy = 0;
    if (KeyDown[XK_j]) 
        dy = PLAYER_SPEED * dt;
    if (KeyDown[XK_k]) 
        dy = -PLAYER_SPEED * dt;
    if (KeyDown[XK_l]) 
        dx = PLAYER_SPEED * dt;
    if (KeyDown[XK_h]) 
        dx = -PLAYER_SPEED * dt;
    
    switch (keypress) {
        case 0:
            dy += (GRAVITY * dt);
            break;

        case XK_space:
            shoot(&self->shots, self->player);
            break;
    }
    self->player.y += (int) dy;
    self->player.x += (int) dx;

    if (rand() % 9000 == 500)
        spawn_asteroids(&self->asteroids, xorg.window);

    update_shots(&self->shots, dt);

    // update asteroids if collision with player change_state and if collision with laser free asteroid
    if (update_asteroids(&self->asteroids, self->shots, self->player, xorg, dt)) 
    {
        free_obj(self->asteroids);
        free_obj(self->shots);
        return change_state(xorg);
    }
        
    if (self->player.y < 0) 
        self->player.y = 0;
    else if (self->player.y + self->player.height > xorg.window.height)
        self->player.y = xorg.window.height - self->player.height;
    
    if (self->player.x < 0)
        self->player.x = 0;
    else if (self->player.x + self->player.width > xorg.window.width)
        self->player.x = xorg.window.width - self->player.width;
}


void play_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;
    render_shots(self->shots, self->gc, xorg);
    render_asteroids(self->asteroids, self->gc, xorg);

    int player_color = PLAYER_COLOR;
    xcb_change_gc(xorg.connection, self->gc, XCB_GC_FOREGROUND, &player_color);
    xcb_void_cookie_t cookie = xcb_poly_fill_rectangle(xorg.connection, 
                                                       xorg.window.id, 
                                                       self->gc, 1, 
                                                       &self->player); 
    xcb_request_check(xorg.connection, cookie);
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

    while (objects) 
    {
        obj *tmp = objects->next;
        free(objects);
        objects = tmp;
    }
}

