#pragma once 

#define translate_y(window, y) ((y * window.h) / VH)
#define translate_x(window, x) ((x * window.w) / VW) 

#define BORDERW 20

#ifdef linux 
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
    int w;
    int h;
    int *buffer;
    xcb_pixmap_t pix;
    xcb_shm_segment_info_t info;
} v_window_t;

xcb_rectangle_t translate_rect_pos(v_window_t window, rectangle_t rect);

window_t window_init(xcb_connection_t *c, xcb_screen_t *s, int w, int h, char *title);
void window_free(xcb_connection_t *c, window_t window);

v_window_t virtual_window_init(xcb_connection_t *c, 
                                xcb_screen_t *s, 
                                xcb_window_t win, int w, int h);
void free_v_window(v_window_t v_window, xcb_connection_t *con);

#else 

typedef struct {
    int x;
    int y;
    int width;
    int height;
    char *title;
    HWND id;
    HDC hdc;
    WNDCLASSW class;
} window_t;

typedef struct { 
    int w;
    int h;
    int *buffer;
    HDC hdc;
} v_window_t;

void window_free(window_t window);
window_t window_init(void (*proc), HINSTANCE hinst, char*title, int w, int h);

void free_v_window(v_window_t window);
v_window_t virtual_window_init(HDC hdc, int w, int h);
RECT translate_rect_pos(v_window_t window, rectangle_t rect);
#endif



