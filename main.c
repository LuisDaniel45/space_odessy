#include <iso646.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <xcb/xproto.h>

#include "states/global.h"
#include "states/states.h"

#define BORDERW 20
#define SECONDS 1000000

void change_state(x11_t xorg)
{
    free(state_machine[cur_state].self);

    cur_state++;
    if (cur_state >= number_of_states) 
        cur_state = 0;

    state_machine[cur_state].load(xorg);
    xcb_clear_area(xorg.connection, 0, 
                   xorg.window.id, 0, 0, 
                   xorg.window.width, 
                   xorg.window.height);
} 


x11_t global_init();
window_t window_init(x11_t);
int *map_keyboard(x11_t xorg, char KeyDown[]);

int main(int argc, char *argv[])
{
    x11_t xorg = global_init();

    xcb_flush(xorg.connection);
    char KeyDown[255];
    int *keyboard = map_keyboard(xorg, KeyDown);

    state_machine[cur_state].load(xorg);

    double dt;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int counter = 0;

    xcb_generic_event_t *event;
    while (1) 
    {
        gettimeofday(&end, NULL);
        if (start.tv_usec < end.tv_usec) 
            dt = (double) ((end.tv_sec - start.tv_sec) +
                 (end.tv_usec - start.tv_usec)) / SECONDS;
        else
            dt = 1.0 / SECONDS;

        gettimeofday(&start, NULL);

        event = xcb_poll_for_event(xorg.connection);
        if (!event)
        {
            state_machine[cur_state].update(xorg, dt, KeyDown, 0);
            xcb_clear_area(xorg.connection, 0, xorg.window.id, 0, 0, xorg.window.width, xorg.window.height);
            state_machine[cur_state].render(xorg);
            continue;
        }
        
        int keypress = 0;
        switch (event->response_type) {
            case XCB_EXPOSE: {
                xcb_expose_event_t *expose = event; 
                xorg.window.x = expose->x;
                xorg.window.y = expose->y;
                xorg.window.width = expose->width;
                xorg.window.height = expose->height;
                continue;
            }

            case XCB_KEY_PRESS: {
                xcb_key_press_event_t *keycode = event;
                keypress = keyboard[keycode->detail];
                if (keypress == XK_q) 
                    goto exit;
                KeyDown[keypress % 255] = 1;
                break;
            }

            case XCB_KEY_RELEASE: {
                xcb_key_press_event_t *keycode = event;
                int value = keyboard[keycode->detail];
                KeyDown[value % 255] = 0;
                break;
            }


            default:
                continue;
        }
        state_machine[cur_state].update(xorg, dt, KeyDown, keypress);
    }

exit:
    xcb_destroy_window(xorg.connection, xorg.window.id);
    xcb_unmap_window(xorg.connection, xorg.window.id);
    xcb_disconnect(xorg.connection);
    free(keyboard);
    return 0;
}

x11_t global_init()
{
    x11_t xorg; 
    xorg.connection = xcb_connect(NULL, NULL);
    xorg.setup      = xcb_get_setup(xorg.connection);
    xorg.screen     = xcb_setup_roots_iterator(xorg.setup).data;
    xorg.window     = window_init(xorg); 
    return xorg;
}

window_t window_init(x11_t xorg)
{
    window_t window = {
        .x = 0, 
        .y = 0,
        .width  = 200, 
        .height = 200, 
        .title  = "Space Odessy",
        .id     = xcb_generate_id(xorg.connection),
        .gc     = xcb_generate_id(xorg.connection)
    };

    // set window config
    int mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    int values[] = {xorg.screen->black_pixel, XCB_EVENT_MASK_KEY_RELEASE | 
                                         XCB_EVENT_MASK_KEY_PRESS   | 
                                         XCB_EVENT_MASK_EXPOSURE    };
    // create window 
    xcb_create_window(xorg.connection,                       // connection  
                      xorg.screen->root_depth,               // window depth 
                      window.id,                        // window id 
                      xorg.screen->root,                     // screen
                      window.x, window.y,               // cordinates of top left corner of window
                      window.width, window.width,       // size of window
                      BORDERW,                          // border width of window
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,    // what the window receives 
                      xorg.screen->root_visual,              // idk  
                      mask, values);                    // the events that program handles 

    xcb_create_gc(xorg.connection, window.gc, window.id, 0, NULL);

    // set window title
    xcb_change_property(xorg.connection, 
                        XCB_PROP_MODE_REPLACE, 
                        window.id, 
                        XCB_ATOM_WM_NAME, 
                        XCB_ATOM_STRING, 8, 
                        strlen(window.title), window.title);

    // map window to screen to make it visible 
    xcb_map_window(xorg.connection, window.id);
    return window;
}

font_t font_init(x11_t xorg, char *name)
{
    font_t font;
    font.id = xcb_generate_id(xorg.connection);

    // open font 
    xcb_open_font(xorg.connection, font.id, strlen(name), name);

    // create graphics context for font 
    font.gc = xcb_generate_id(xorg.connection);
    int gc_mask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_FONT;
    int gc_values[] = {xorg.screen->white_pixel, xorg.screen->black_pixel, font.id};

    xcb_create_gc(xorg.connection, font.gc, xorg.window.id, gc_mask, gc_values);
    return font;
}

int print_screen(x11_t xorg, char *text, font_t font, int x, int y)
{
    xcb_void_cookie_t testCookie = xcb_image_text_8_checked(xorg.connection, strlen(text), 
                                                            xorg.window.id, font.gc, x, y, text);
    return xcb_request_check(xorg.connection, testCookie) ? true : false;
}


int *map_keyboard(x11_t xorg, char KeyDown[])
{
    xcb_get_keyboard_mapping_reply_t* keyboard_mapping = 
        xcb_get_keyboard_mapping_reply(xorg.connection,
            xcb_get_keyboard_mapping(xorg.connection,
                xorg.setup->min_keycode,
                xorg.setup->max_keycode - xorg.setup->min_keycode + 1),
            NULL);

    xcb_keysym_t* keysyms  = (xcb_keysym_t*)(keyboard_mapping + 1);  
                                                                             
    int *keyboard = malloc(sizeof(int) * xorg.setup->max_keycode);
    for (int i = xorg.setup->min_keycode; i < xorg.setup->max_keycode; i++) {
        keyboard[i] = keysyms[0 + (i - xorg.setup->min_keycode) * keyboard_mapping->keysyms_per_keycode];
        KeyDown[keyboard[i] % 255] = 0;
    }

    free(keyboard_mapping);
    return keyboard;
}     
