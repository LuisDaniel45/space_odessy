#include "global.h"
#include "states.h" 
#include "objects/objects.h"
#include <string.h>
#include <xcb/xcb_image.h>


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../stb_image.h"
#include "../stb_image_resize.h"

#define PLAYER_SPEED 100 
#define GRAVITY 50 

typedef struct {
    xcb_gcontext_t gc;
    entity_t player;
    obj *asteroids;
    obj *shots;
} self_t;

void free_obj(obj*);
void render_image(xcb_image_t *window, xcb_image_t *data, int dest_x, int dest_y);

void play_state_load(x11_t xorg)
{
    load_asteroids(xorg);
    state_machine[cur_state].self = malloc(sizeof(self_t)); 
    self_t *self = state_machine[cur_state].self;

    self->gc = xcb_generate_id(xorg.connection);
    xcb_create_gc(xorg.connection, self->gc, xorg.window.id, 0, NULL);

    self->player.skin = load_image("spaceship.png", 0, 0, xorg);
    self->player.pos.width = self->player.skin->width - 20;
    self->player.pos.height= self->player.skin->height - 10;
    self->player.pos.x = VW / 2;
    self->player.pos.y = VH - self->player.pos.height;
    self->player.x_offset = 10;
    self->player.y_offset = 10;

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
    else if (self->player.pos.y + self->player.pos.height > VH)
        self->player.pos.y = VH - self->player.pos.height;
    
    if (self->player.pos.x < 0)
        self->player.pos.x = 0;
    else if (self->player.pos.x + self->player.pos.width > VW)
        self->player.pos.x = VW - self->player.pos.width;
}


void play_state_render(x11_t xorg)
{
    self_t *self = state_machine[cur_state].self;

    memcpy(xorg.window.buffer->data, xorg.window.background, VH *VW *4);
    render_image(xorg.window.buffer, 
                  self->player.skin, 
                  self->player.pos.x - self->player.x_offset, 
                  self->player.pos.y - self->player.y_offset);
    /** render_rectangle(xorg.window.buffer, self->player.pos, 0x000000ff); */

    render_asteroids(self->asteroids, self->gc, xorg);
    render_shots(self->shots, self->gc, xorg);

    int rw = (int) ((float) (xorg.window.height * VW) / VH); 
    unsigned char data[rw * xorg.window.height * 4];
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(xorg.window.buffer->data,
                 VW, VH, 0,
                 data, rw, xorg.window.height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);
    xcb_image_t r = *xorg.window.buffer;
    r.data = data;
    r.size = rw * xorg.window.height * 4;
    r.width = rw;
    r.height = xorg.window.height;

    xcb_request_check(xorg.connection, 
                      xcb_image_put(xorg.connection, 
                                    xorg.window.id, 
                                    self->gc, 
                                    &r, (xorg.window.width / 2) - (rw/2), 
                                    0, 0));
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

    while (objects) {
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
                                       data, w * h * 4, data);
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

void render_rectangle(xcb_image_t *window, xcb_rectangle_t rect, int color)
{
    for (int i = rect.y; i < rect.height + rect.y && i < VH; i++) {
        xcb_image_put_pixel(window, rect.x, i, color);
        xcb_image_put_pixel(window, rect.x + rect.width, i, color);
    }

    for (int i = rect.x; i < rect.width + rect.x && i < VW; i++) {
        xcb_image_put_pixel(window, i, rect.y, color);
        xcb_image_put_pixel(window, i, rect.y + rect.height, color);
    }
}

void render_fill_rectangle(xcb_image_t *window, xcb_rectangle_t rect, int color)
{
    for (int y = rect.y; y < rect.y + rect.height && y < VH; y++) 
        for (int x = rect.x; x < rect.x + rect.width && x < VW; x++) 
            xcb_image_put_pixel(window, x, y, color);
}

void render_image(xcb_image_t *window, xcb_image_t *image, int dest_x, int dest_y)
{
    for (int y = dest_y; y < dest_y + image->height && y < window->height; y++) {
        for (int x = dest_x; x < dest_x + image->width && x < window->width; x++) {
            unsigned char *pixel = image->data + ((x - dest_x) + ((y - dest_y) * image->width)) * 4;
            if (pixel[3] == 0) 
                continue;
            xcb_image_put_pixel(window, x, y, *(unsigned int*) pixel);
        }
    
    }
}
