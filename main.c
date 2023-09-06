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

void change_state()
{
    if (cur_state == END_STATE) 
        return;
    cur_state++;
    state_machine[cur_state].load();
    xcb_clear_area(connection, 0, window.id, 0, 0, window.width, window.height);
} 


void global_init();
window_t window_init();
int *map_keyboard(const xcb_setup_t* setup);

int main(int argc, char *argv[])
{
    global_init();

    xcb_flush(connection);
    int *keyboard = map_keyboard(xcb_get_setup(connection));

    state_machine[cur_state].load();

    double dt;
    struct timeval start, end;
    gettimeofday(&start, NULL);

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

        event = xcb_poll_for_event(connection);
        if (!event)
        {
            state_machine[cur_state].update(dt, 0);
            xcb_clear_area(connection, 0, window.id, 0, 0, window.width, window.height);
            state_machine[cur_state].render();
            continue;
        }
        
        switch (event->response_type) {
            case XCB_EXPOSE: {
                xcb_expose_event_t *expose = event; 
                window.x = expose->x;
                window.y = expose->y;
                window.width = expose->width;
                window.height = expose->height;

                state_machine[cur_state].render();
                break;
            }

            case XCB_KEY_PRESS: {
                xcb_key_press_event_t *keycode = event;
                int keypress = keyboard[keycode->detail];
                if (keypress == XK_q) 
                    goto exit;

                xcb_clear_area(connection, 0, window.id, 0, 0, window.width, window.height);
                state_machine[cur_state].update(dt, keypress);
                state_machine[cur_state].render();
                break;
            }


            default:
                continue;
        }
    }

exit:
    xcb_unmap_window(connection, window.id);
    xcb_destroy_window(connection, window.id);
    xcb_disconnect(connection);
    free(keyboard);
    return 0;
}

void global_init()
{
    connection = xcb_connect(NULL, NULL);
    screen     = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    window     = window_init(); 
}

window_t window_init()
{
    window_t window = {
        .x = 0, 
        .y = 0,
        .width  = 200, 
        .height = 200, 
        .title  = "Manga",
        .id     = xcb_generate_id(connection),
        .gc     = xcb_generate_id(connection)
    };

    // set window config
    int mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    int values[] = {screen->black_pixel, XCB_EVENT_MASK_KEY_RELEASE | 
                                         XCB_EVENT_MASK_KEY_PRESS   | 
                                         XCB_EVENT_MASK_EXPOSURE    };
    // create window 
    xcb_create_window(connection,                       // connection  
                      screen->root_depth,               // window depth 
                      window.id,                        // window id 
                      screen->root,                     // screen
                      window.x, window.y,               // cordinates of top left corner of window
                      window.width, window.width,       // size of window
                      BORDERW,                          // border width of window
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,    // what the window receives 
                      screen->root_visual,              // idk  
                      mask, values);                    // the events that program handles 

    xcb_create_gc(connection, window.gc, window.id, 0, NULL);

    // set window title
    xcb_change_property(connection, 
                        XCB_PROP_MODE_REPLACE, 
                        window.id, 
                        XCB_ATOM_WM_NAME, 
                        XCB_ATOM_STRING, 8, 
                        strlen(window.title), window.title);

    // map window to screen to make it visible 
    xcb_map_window(connection, window.id);
    return window;
}

font_t font_init(char *name)
{
    font_t font;
    font.id = xcb_generate_id(connection);

    // open font 
    xcb_open_font(connection, font.id, strlen(name), name);

    // create graphics context for font 
    font.gc = xcb_generate_id(connection);
    int gc_mask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_FONT;
    int gc_values[] = {screen->white_pixel, screen->black_pixel, font.id};

    xcb_create_gc(connection, font.gc, window.id, gc_mask, gc_values);
    return font;
}

int print_screen(char *text, font_t font, int x, int y)
{
    xcb_void_cookie_t testCookie = xcb_image_text_8_checked(connection, strlen(text), 
                                                            window.id, font.gc, x, y, text);
    return xcb_request_check(connection, testCookie) ? true : false;
}


int *map_keyboard(const xcb_setup_t* setup)
{
    xcb_get_keyboard_mapping_reply_t* keyboard_mapping = 
        xcb_get_keyboard_mapping_reply(connection,
            xcb_get_keyboard_mapping(connection,
                setup->min_keycode,
                setup->max_keycode - setup->min_keycode + 1),
            NULL);

    xcb_keysym_t* keysyms  = (xcb_keysym_t*)(keyboard_mapping + 1);  
                                                                             
    int *keyboard = malloc(sizeof(int) * setup->max_keycode);
    for (int i = setup->min_keycode; i < setup->max_keycode; i++) 
        keyboard[i] = keysyms[0 + (i - setup->min_keycode) * keyboard_mapping->keysyms_per_keycode];

    free(keyboard_mapping);
    return keyboard;
}     
