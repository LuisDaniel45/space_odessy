#include "../global.h"

typedef struct {
    int x_offset;
    int y_offset;
    xcb_rectangle_t pos;
    xcb_image_t *skin;
} entity_t;

typedef struct node {
    entity_t entity;
    struct node *next;
} obj;

// shooting mechanics 
#define SHOT_SPEED 400 
#define SHOT_COLOR 0x00ff0000
#define MAX_SKINS 5
void shoot(obj **, xcb_rectangle_t);
void update_shots(obj**, double);
void render_shots(obj*, xcb_gcontext_t,  x11_t);
char shot_collision(obj*, xcb_rectangle_t );

// asteroids mechanics 
#define ASTEROIDS_SPEED 300 
void load_asteroids(x11_t);
void spawn_asteroids(x11_t, obj**, window_t);
char update_asteroids(obj **, obj *, xcb_rectangle_t, x11_t, double);
void render_asteroids(obj*, xcb_gcontext_t, x11_t);

char collision(xcb_rectangle_t a, xcb_rectangle_t b);
xcb_image_t *load_image(char *file, int width, int height, x11_t xorg);
xcb_image_t *resize_image(xcb_image_t *image, int width, int height);

void render_image(xcb_image_t *window, xcb_image_t *data, int dest_x, int dest_y);
void render_rectangle(xcb_image_t *window, xcb_rectangle_t rect, int color);
void render_fill_rectangle(xcb_image_t *window, xcb_rectangle_t rect, int color);
