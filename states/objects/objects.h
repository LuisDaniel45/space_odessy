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
void shoot(obj **, xcb_rectangle_t);
void update_shots(obj**, double);
void render_shots(obj*, xcb_gcontext_t,  x11_t);
char shot_collision(obj*, xcb_rectangle_t );

// asteroids mechanics 
#define ASTEROIDS_SPEED 300 
#define ASTEROIDS_COLOR  0x00939597
void load_asteroids(x11_t);
void spawn_asteroids(x11_t, obj**, window_t);
char update_asteroids(obj **, obj *, xcb_rectangle_t, x11_t, double);
void render_asteroids(obj*, xcb_gcontext_t, x11_t);

char collision(xcb_rectangle_t a, xcb_rectangle_t b);
xcb_image_t *load_image(char *file, int width, int height, x11_t xorg);
xcb_image_t *resize_image(xcb_image_t *image, int width, int height);

