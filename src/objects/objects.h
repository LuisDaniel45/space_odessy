#include "../global.h"

typedef struct {
    int x_offset;
    int y_offset;
    xcb_rectangle_t pos;
    image_t skin;
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
void render_shots(obj *shots, x11_t xorg, xcb_gcontext_t gc);
char shot_collision(obj*, xcb_rectangle_t );

// asteroids mechanics 
#define ASTEROIDS_SPEED 300 
void load_asteroids(x11_t);
void spawn_asteroids(x11_t xorg, obj **asteroids);
char update_asteroids(obj **, obj *, xcb_rectangle_t, x11_t, double);
void render_asteroids(obj *asteroids, v_window_t window);
void unload_asteroids(obj *asteroids);

char collision(xcb_rectangle_t a, xcb_rectangle_t b);
void free_obj(obj *objects);

