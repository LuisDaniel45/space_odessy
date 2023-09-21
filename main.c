#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "states/global.h"
#include "states/states.h"

#define BORDERW 20
#define SECONDS 1000000


x11_t global_init();
window_t window_init(x11_t);
void change_state(x11_t xorg);
int *map_keyboard(x11_t xorg, char KeyDown[]);
void resize_bg(x11_t xorg, xcb_pixmap_t *pixmap, int width, int height);
pixmap_t pixmap_init(x11_t xorg, int w, int h);
void free_pixmap(pixmap_t pix, xcb_connection_t *con);

int main(int argc, char *argv[])
{
    x11_t xorg = global_init();

    xcb_flush(xorg.connection);
    char KeyDown[255];
    int *keyboard = map_keyboard(xorg, KeyDown);

    state_machine[cur_state].load(xorg);

    double time;
    int counter = 0;
    double dt = 100.0 / SECONDS;
    const int max_updates = 1; 
    const long time_per_frame = SECONDS / 30;
    struct timeval start, end;
    gettimeofday(&start, NULL);

    xcb_generic_event_t *event;
    while (1) 
    {
        if (counter > max_updates) 
        {
            dt = (double) time / SECONDS;
            if (time_per_frame > time) 
                usleep(time_per_frame - time);

            xcb_request_check(xorg.connection, 
                xcb_copy_area(
                    xorg.connection, 
                    xorg.window.bg, 
                    xorg.v_window.pix, 
                    xorg.window.gc, 
                    0, 0, 0, 0, 
                    xorg.v_window.w, 
                    xorg.v_window.h
                )
            );

            state_machine[cur_state].render(xorg);

            xcb_request_check(xorg.connection, 
                xcb_copy_area(
                    xorg.connection, 
                    xorg.v_window.pix, 
                    xorg.window.id, 
                    xorg.window.gc, 
                    0, 0, 
                    (xorg.window.width/2) - (xorg.v_window.w/2), 
                    0,
                    xorg.v_window.w, xorg.v_window.h
                )
            );


            counter = 0;
            time = 0;
        }

        gettimeofday(&end, NULL);

        time += (((end.tv_sec - start.tv_sec) * SECONDS) +
                 (end.tv_usec - start.tv_usec));

        start = end;

        int keypress = 0;
        while ((event = xcb_poll_for_event(xorg.connection))) 
        {
            switch (event->response_type) {
                case XCB_EXPOSE: {
                    xcb_expose_event_t *expose = event; 
                    xorg.window.x = expose->x;
                    xorg.window.y = expose->y;
                    xorg.window.width = expose->width;
                    xorg.window.height = expose->height;

                    int rw = (int) ((float) (expose->height * VW) / VH);
                    resize_bg(xorg, &xorg.window.bg, rw, expose->height);
                    free_pixmap(xorg.v_window, xorg.connection);
                    xorg.v_window = pixmap_init(xorg, rw, expose->height);

                    xcb_clear_area(xorg.connection, 
                                   0, xorg.window.id, 
                                   0, 0, 
                                   expose->width, expose->height);
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
            }
        }

        state_machine[cur_state].update(xorg, dt, KeyDown, keypress);
        counter++; 
    }

exit:
    free_pixmap(xorg.v_window, xorg.connection);
    xcb_free_pixmap(xorg.connection, xorg.v_window.pix);
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
    xorg.v_window   = pixmap_init(xorg, VW, VH);
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
        .gc     = xcb_generate_id(xorg.connection),
    };

    // set window config
    int mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    int values[] = {0x00010114, XCB_EVENT_MASK_KEY_RELEASE | 
                                XCB_EVENT_MASK_KEY_PRESS   | 
                                XCB_EVENT_MASK_EXPOSURE    };

    // create window 
    xcb_create_window(xorg.connection,                  // connection  
                      xorg.screen->root_depth,          // window depth 
                      window.id,                        // window id 
                      xorg.screen->root,                // screen
                      window.x, window.y,               // cordinates of top left corner of window
                      window.width, window.width,       // size of window
                      BORDERW,                          // border width of window
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,    // what the window receives 
                      xorg.screen->root_visual,         // idk  
                      mask, values);                    // the events that program handles 

    mask = XCB_GC_FOREGROUND;
    int values_gc[] = {0x00000000};
    xcb_create_gc(xorg.connection, window.gc, window.id, mask, &values_gc);

    window.bg = xcb_generate_id(xorg.connection); 
    xcb_create_pixmap(xorg.connection, 
                      xorg.screen->root_depth, 
                      window.bg, 
                      window.id, 
                      VW, VH);

    xcb_poly_fill_rectangle(
          xorg.connection, 
          window.bg, 
          window.gc, 1, 
          (xcb_rectangle_t[]) {{
                .x = 0, 
                .y = 0, 
                .width = VW, 
                .height = VH 
          }}
    );

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
                                                            xorg.v_window.pix, font.gc, x, y, text);
    return xcb_request_check(xorg.connection, testCookie) ? true : false;
}

void resize_bg(x11_t xorg, xcb_pixmap_t *pixmap, int width, int height)
{
    xcb_free_pixmap(xorg.connection, *pixmap);
    *pixmap = xcb_generate_id(xorg.connection);
    xcb_create_pixmap(xorg.connection, xorg.screen->root_depth, *pixmap, xorg.window.id, width, height);
    xcb_poly_fill_rectangle(
          xorg.connection, 
          *pixmap, 
          xorg.window.gc, 1, 
          (xcb_rectangle_t[]) {{
                .x = 0, 
                .y = 0, 
                .width = width, 
                .height = height
          }}
    );
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

void change_state(x11_t xorg)
{
    free(state_machine[cur_state].self);

    cur_state++;
    if (cur_state >= number_of_states) 
        cur_state = 0;

    state_machine[cur_state].load(xorg);
} 

void render_image(pixmap_t window, xcb_image_t *image, int dest_x, int dest_y)
{
    int ndx = (dest_x * window.w) / VW;
    int ndy = (dest_y * window.h) / VH;

    int xo = 0, yo = 0;
    if (dest_y < 0) 
        yo = ndy;
    if (dest_x < 0) 
        xo = ndx; 

    int nw = (image->width * window.w) / VW;
    int nh = (image->height * window.h) / VH;

    double x_scale = (double)image->width / nw; 
    double y_scale = (double)image->height / nh; 

    for (int y = ndy - yo; y < ndy + nh && y < window.h; y++) {
        for (int x = ndx - xo; x < ndx + nw && x < window.w; x++) {
           int src_x = (int) ((x - ndx) * x_scale);
           int src_y = (int) ((y - ndy) * y_scale);

            unsigned char *pixel = &image->data[(src_x + (src_y * image->width)) * 4];
            if (pixel[3] == 0)
                continue;
            window.buffer[x + (y * window.w)] = *(int*)pixel;
        }

    }
}

pixmap_t pixmap_init(x11_t xorg, int w, int h)
{
    pixmap_t result; 
    xcb_shm_query_version_reply_t*  reply;

    reply = xcb_shm_query_version_reply(
                xorg.connection,
                xcb_shm_query_version(xorg.connection),
                NULL
            );

    if(!reply || !reply->shared_pixmaps)
    {
        printf("Shm error...\n");
        exit(0);
    }

    result.info.shmid   = shmget(IPC_PRIVATE, w*h*4, IPC_CREAT | 0777);
    result.info.shmaddr = shmat(result.info.shmid, 0, 0);

    result.info.shmseg = xcb_generate_id(xorg.connection);
    xcb_shm_attach(xorg.connection, result.info.shmseg, result.info.shmid, 0);
    shmctl(result.info.shmid, IPC_RMID, 0);

    result.buffer = result.info.shmaddr;

    result.pix = xcb_generate_id(xorg.connection);
    xcb_shm_create_pixmap(
        xorg.connection,
        result.pix,
        xorg.window.id,
        w, h,
        xorg.screen->root_depth,
        result.info.shmseg,
        0
    );
    result.w = w;
    result.h = h;

    return result;
}

void free_pixmap(pixmap_t pix, xcb_connection_t *con)
{
    xcb_shm_detach(con, pix.info.shmseg);
    shmdt(pix.info.shmaddr);
    xcb_free_pixmap(con, pix.pix);
}
