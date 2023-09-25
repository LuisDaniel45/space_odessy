#pragma once 

#define BORDERW 20

typedef struct {
    int x;
    int y;
    int width;
    int height;
    char *title;
    xcb_window_t id;
    xcb_gcontext_t gc;
} window_t;

typedef struct { 
    xcb_pixmap_t pix;
    int *buffer;
    xcb_shm_segment_info_t info;
    int w;
    int h;
} v_window_t;

xcb_rectangle_t translate_rect_pos(v_window_t window, xcb_rectangle_t rect);
int translate_x(v_window_t window, int x);
int translate_y(v_window_t window, int y);

window_t window_init(xcb_connection_t *c, xcb_screen_t *s);
void window_free(xcb_connection_t *c, window_t window);

v_window_t virtual_window_init(xcb_connection_t *c, 
                               xcb_screen_t *s, 
                               xcb_window_t win, int w, int h);
void free_v_window(v_window_t v_window, xcb_connection_t *con);
