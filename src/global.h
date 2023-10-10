#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VW 300 
#define VH 600

#ifdef linux 

#include <sys/types.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/keysym.h>
#include <xcb/xcb_image.h>
#else /* end of linux */

#include <windows.h> 
#include <wingdi.h>
#endif /* end of WIN32 */

#define key_pressed(keyboard, keycode) keyboard[keycode] |= 1 << 31
#define key_release(keyboard, keycode) keyboard[keycode] = (keyboard[keycode] << 1) >> 1
#define is_key_down(table, key)  ((*table[key]) >> 31)
enum keys_t {
    KEY_unmap,
    KEY_quit,
    KEY_left,
    KEY_down,
    KEY_up,
    KEY_right,
    KEY_shoot,
    KEY_select,
    KEY_retry,
    KEY_b,
    KEY_MAX
};

#ifdef linux 

static int keys_table[] = {
    0,
    XK_q,
    XK_h,
    XK_j,
    XK_k,
    XK_l,
    XK_space,
    XK_Return,
    XK_r,
    XK_b
};
#else /* end of linux */

static int keys_table[] = {
     0,
    'Q',
    'H',
    'J',
    'K',
    'L',
    VK_SPACE,
    VK_RETURN,
    'R',
    'B'
};
#endif /* end of WIN32 */


typedef struct {
    int x;
    int y;
    int width;
    int height;
} rectangle_t;

#ifdef linux
typedef xcb_gcontext_t color_t;
#else /* end of linux */ 
typedef HBRUSH color_t;
#endif /* end of WIN32 */

#include "util/window.h"
#include "util/image.h"
#include "util/sound.h"
#include "util/font.h"

#ifdef linux 
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
    sound_t sounds[SOUND_MAX];
    font_t font;
} global_t;


#else /* end of linux */

typedef struct {
    int y;
    int cur_height;
    image_t image;
    HDC hdc;
} background_t;

typedef struct {
    window_t window;
    v_window_t v_window;
    background_t bg;
    image_t textures;
    sound_t sounds[SOUND_MAX];
    font_t font;
} global_t;

#endif /* end of WIN32 */

void color_free(color_t color, void *arg);
color_t create_color(int color, void *arg);
void render_rectangle(global_t g, rectangle_t rect, color_t color);
