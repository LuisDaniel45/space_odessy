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

#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

enum  sounds_enum {
    SOUND_LAUNCH,
    SOUND_SHOOT,
    SOUND_BREAK,
    SOUND_GAME_OVER,
    SOUND_SELECT,
    SOUND_MAX
}; 

typedef struct {
    ALuint source;
    ALuint buffer;
} sound_element_t;

typedef struct {
    ALCcontext *context;
    ALCdevice *device;
    sound_element_t sounds[SOUND_MAX];
} sounds_t;

typedef struct { 
    int width;
    int height;
    int y_offset;
    unsigned char *data;
} image_t;

typedef struct {
    int y;
    int cur_height;
    xcb_pixmap_t pixmap;
    image_t image;
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
    image_t textures;
    sounds_t sounds;
} x11_t;

typedef struct {
    FT_Face face;
    FT_Library ft;
    int w;
    int h;
} font_t;

#define VW 300 
#define VH 600

void sound_play(sounds_t sound, enum sounds_enum i);
void sound_pause(sounds_t sound, enum sounds_enum i);
char isSoundPlaying(sounds_t sound, enum sounds_enum i);
sounds_t sound_init();

void free_font(font_t font);
int font_init(char *name, font_t *font);
int render_text(v_window_t window, font_t font, char *str, int dest_x, int dest_y, int color);

void render_image(v_window_t window, image_t image, int dest_x, int dest_y);
image_t load_image(char *file, int width, int height, x11_t xorg);
image_t resize_image(image_t image, int width, int height);
image_t slice_texture(image_t texture, int x, int y, int w, int h, char cpy);
image_t flip_image(image_t image);

xcb_rectangle_t translate_rect_pos(v_window_t window, xcb_rectangle_t rect);
int translate_x(v_window_t window, int x);
int translate_y(v_window_t window, int y);
