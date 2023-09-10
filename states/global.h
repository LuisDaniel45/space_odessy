#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <xcb/xcb.h>
#include <string.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>

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
    xcb_setup_t *setup;
} x11_t;

typedef struct {
    xcb_font_t id;
    xcb_gcontext_t gc;
} font_t;

// // global variables
// static xcb_connection_t *connection;
// static xcb_screen_t     *screen;
// static window_t          window;

void change_state(x11_t xorg);
int  print_screen(x11_t xorg, char *text, font_t font, int x, int y); 
font_t font_init(x11_t xorg, char *name);



