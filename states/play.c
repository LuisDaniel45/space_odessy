#include "states.h" 
#include "objects/objects.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xproto.h>


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../stb_image.h"
#include "../stb_image_resize.h"

#define PLAYER_SPEED 100 
#define PLAYER_COLOR 0x000000ff

#define GRAVITY 50 

typedef struct {
    xcb_gcontext_t gc;
    entity_t player;
    obj *asteroids;
    obj *shots;
} self_t;

void free_obj(obj*);

void play_state_load(x11_t xorg)
{
    load_asteroids(xorg);
    state_machine[cur_state].self = malloc(sizeof(self_t)); 
    self_t *self = state_machine[cur_state].self;
    self->gc = xcb_generate_id(xorg.connection);
    xcb_create_gc(xorg.connection, self->gc, xorg.window.id, 0, NULL);

    self->player.pos.width = 100;
    self->player.pos.height= 170;
    self->player.pos.x = xorg.window.width / 2;
    self->player.pos.y = xorg.window.height - self->player.pos.height;
    self->player.x_offset = 50;
    self->player.y_offset = 20;
    self->player.skin = load_image("spaceship.png", 200, 200, xorg);

    self->shots = NULL;
    self->asteroids = NULL;
}

void play_state_update(x11_t xorg, double dt, char KeyDown[], int keypress)
{
    self_t *self = state_machine[cur_state].self;
    double dx = 0, dy = 0;
    if (KeyDown[XK_j]) 
        dy = PLAYER_SPEED * dt;
    else if (KeyDown[XK_k]) 
        dy = -PLAYER_SPEED * dt;
    if (KeyDown[XK_l]) 
        dx = PLAYER_SPEED * dt;
    else if (KeyDown[XK_h]) 
        dx = -PLAYER_SPEED * dt;
    
    switch (keypress) {
        case 0:
            dy += GRAVITY * dt;
            break;

        case XK_space:
            shoot(&self->shots, self->player.pos);
            break;
    }
    self->player.pos.y = (int) round((double) self->player.pos.y  + dy);
    self->player.pos.x = (int) round((double) self->player.pos.x + dx);

    if (rand() % 100 == 0)
        spawn_asteroids(xorg, &self->asteroids, xorg.window);

    update_shots(&self->shots, dt);

    // update asteroids if collision with player change_state and if collision with laser free asteroid
    if (update_asteroids(&self->asteroids, self->shots, self->player.pos, xorg, dt)) 
    {
        free_obj(self->asteroids);
        free_obj(self->shots);
        xcb_image_destroy(self->player.skin);
        return change_state(xorg);
    }
        
    if (self->player.pos.y < 0) 
        self->player.pos.y = 0;
    else if (self->player.pos.y + self->player.pos.height > xorg.window.height)
        self->player.pos.y = xorg.window.height - self->player.pos.height;
    
    if (self->player.pos.x < 0)
        self->player.pos.x = 0;
    else if (self->player.pos.x + self->player.pos.width > xorg.window.width)
        self->player.pos.x = xorg.window.width - self->player.pos.width;
}


void play_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;
    render_shots(self->shots, self->gc, xorg);

    xcb_void_cookie_t cookie = xcb_image_put(xorg.connection, 
                                             xorg.window.id, 
                                             self->gc, self->player.skin, 
                                             self->player.pos.x - self->player.x_offset, 
                                             self->player.pos.y - self->player.y_offset, 0);

    render_asteroids(self->asteroids, self->gc, xorg);

    /** int player_color = PLAYER_COLOR; */
    /** xcb_change_gc(xorg.connection, self->gc, XCB_GC_FOREGROUND, &player_color); */
    /** xcb_void_cookie_t cookie = xcb_poly_fill_rectangle(xorg.connection,  */
    /**                                                    xorg.window.id,  */
    /**                                                    self->gc, 1,  */
    /**                                                    &self->player);  */
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

xcb_image_t *resize_image(xcb_image_t *image, int width, int height)
{
    int size = width * height * 4;
    unsigned char *resized_data = malloc(size);
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(image->data,
                 image->width, image->height, 0,
                 resized_data, width, height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);

    xcb_image_t *image_out = malloc(sizeof(xcb_image_t));
    *image_out = *image;
    image_out->size = size;
    image_out->width = width;
    image_out->height = height;
    image_out->data = resized_data;

    return image_out;
}

xcb_image_t *load_image(char *file, int width, int height, x11_t xorg)
{
    int channels, size, w, h;
    unsigned char *data = stbi_load(file, &w, &h, &channels, STBI_rgb_alpha);
    for (int i = 0; i < w * h; i++) {
        unsigned char *pixel = data + (i * channels);
        char tmp = *pixel;
        *pixel  = pixel[2];
        pixel[2] = tmp;
    }

    if (!width || !height) 
        return xcb_image_create_native(xorg.connection, 
                                       w, h, 
                                       XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                       xorg.screen->root_depth, 
                                       data, size, data);
        
    

    size = width * height * channels;

    unsigned char *resized_data = malloc(size);
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(data,
                 w, h, 0,
                 resized_data, width, height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);
    stbi_image_free(data);

    return xcb_image_create_native(xorg.connection, 
                                   width, height, 
                                   XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                   xorg.screen->root_depth, 
                                   resized_data, size, resized_data);
}
