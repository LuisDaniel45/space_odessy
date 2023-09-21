#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>
#include <xcb/xcb_image.h>

#include <xcb/shm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
    int y;
    int cur_height;
    xcb_pixmap_t pixmap;
    xcb_image_t *image;
} background_t;

typedef struct { 
    xcb_pixmap_t pix;
    int *buffer;
    xcb_shm_segment_info_t info;
    int w;
    int h;
} v_window_t;

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
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    window_t window;
    v_window_t v_window;
    xcb_setup_t *setup;
    background_t bg;
} x11_t;

typedef struct {
    xcb_font_t id;
    xcb_gcontext_t gc;
} font_t;

// // global variables
// static xcb_connection_t *connection;
// static xcb_screen_t     *screen;
// static window_t          window;

#define VW 300 
#define VH 600


int  print_screen(x11_t xorg, char *text, font_t font, int x, int y); 
font_t font_init(x11_t xorg, char *name);

void render_image(v_window_t window, xcb_image_t *image, int dest_x, int dest_y);
xcb_image_t *load_image(char *file, int width, int height, x11_t xorg);
xcb_image_t *resize_image(xcb_image_t *image, int width, int height);
