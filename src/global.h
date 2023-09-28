#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define VW 300 
#define VH 600

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>
#include <xcb/xcb_image.h>

#include "util/window.h"
#include "util/image.h"
#include "util/sound.h"
#include "util/font.h"


typedef struct {
    int y;
    int cur_height;
    xcb_pixmap_t pixmap;
    image_t image;
} background_t;


typedef struct {
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    window_t window;
    v_window_t v_window;
    xcb_setup_t *setup;
    background_t bg;
    image_t textures;
    sounds_t sounds;
    font_t font;
} x11_t;

