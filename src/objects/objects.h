#include "../global.h"

typedef struct {
    int x_offset;
    int y_offset;
    rectangle_t pos;
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
void shoot(obj **, rectangle_t);
void update_shots(obj**, double);
void render_shots(obj *shots, global_t g, color_t color);
char shot_collision(obj*, rectangle_t );

// asteroids mechanics 
#define ASTEROIDS_SPEED 300 
void load_asteroids(global_t);
void spawn_asteroids(global_t xorg, obj **asteroids);
char update_asteroids(obj **asteroids, obj *shots, rectangle_t player, global_t *g, double dt);
void render_asteroids(obj *asteroids, v_window_t window);
void unload_asteroids(obj *asteroids);

char collision(rectangle_t a, rectangle_t b);
void free_obj(obj *objects);

