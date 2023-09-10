#include "../global.h"


typedef struct node {
    xcb_rectangle_t pos;
    struct node *next;
} obj;

// shooting mechanics 
#define SHOT_SPEED 1000 
#define SHOT_COLOR 0x00ff0000
void shoot(obj **shots, xcb_rectangle_t);
void update_shots(obj**, double);
void render_shots(obj*, xcb_gcontext_t,  x11_t);
char shot_collision(obj *, xcb_rectangle_t );

// asteroids mechanics 
#define ASTEROIDS_SPEED 300 
#define ASTEROIDS_COLOR  0x00939597
void spawn_asteroids(obj**, window_t);
char update_asteroids(obj **, obj *, xcb_rectangle_t, x11_t, double );
void render_asteroids(obj*, xcb_gcontext_t, x11_t);

char collision(xcb_rectangle_t a, xcb_rectangle_t b);

