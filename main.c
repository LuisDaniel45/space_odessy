#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <xcb/xcb.h>

#include "states/global.h"
#include "states/states.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"


#define BORDERW 20
#define SECONDS 1000000
#define BG_SPEED 100


x11_t        global_init();
window_t     window_init(x11_t);
v_window_t   virtual_window_init(x11_t xorg, int w, int h);
background_t background_init(x11_t xorg);

int *map_keyboard(x11_t xorg, char KeyDown[]);
void resize_bg(x11_t xorg, background_t *bg, int width, int height);
void free_v_window(v_window_t v_window, xcb_connection_t *con);

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

            xcb_void_cookie_t cookie;

            if (xorg.bg.y < xorg.v_window.h) 
            { 
                cookie = xcb_copy_area(xorg.connection,
                        xorg.bg.pixmap,
                        xorg.v_window.pix,
                        xorg.window.gc,
                        0, xorg.bg.cur_height - xorg.bg.y, 0, 0,
                        xorg.v_window.w,
                        xorg.bg.y);
                xcb_copy_area(xorg.connection,
                        xorg.bg.pixmap,
                        xorg.v_window.pix,
                        xorg.window.gc,
                        0, 0, 0, xorg.bg.y,
                        xorg.v_window.w,
                        xorg.v_window.h - xorg.bg.y);
            }
            else 
                cookie = xcb_copy_area(xorg.connection,
                        xorg.bg.pixmap,
                        xorg.v_window.pix,
                        xorg.window.gc,
                        0, xorg.bg.cur_height - xorg.bg.y, 0, 0,
                        xorg.v_window.w,
                        xorg.v_window.h);

            xcb_request_check(xorg.connection, cookie);

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
                    resize_bg(xorg, &xorg.bg, rw, expose->height);
                    free_v_window(xorg.v_window, xorg.connection);
                    xorg.v_window = virtual_window_init(xorg, rw, expose->height);

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
        xorg.bg.y = (xorg.bg.y + (int)(BG_SPEED * dt)) % xorg.bg.cur_height;
        counter++; 
    }

exit:
    free_v_window(xorg.v_window, xorg.connection);
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
    xorg.v_window   = virtual_window_init(xorg, VW, VH);
    xorg.bg         = background_init(xorg);
    return xorg;
}

window_t window_init(x11_t xorg)
{
    window_t window = {
        .x = 0, 
        .y = 0,
        .width  = VW * 2, 
        .height = VH * 1.5, 
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
                      window.width, window.height,      // size of window
                      BORDERW,                          // border width of window
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,    // what the window receives 
                      xorg.screen->root_visual,         // idk  
                      mask, values);                    // the events that program handles 

    mask = XCB_GC_FOREGROUND;
    int values_gc[] = {0x00000000};
    xcb_create_gc(xorg.connection, window.gc, window.id, mask, &values_gc);

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

void resize_bg(x11_t xorg, background_t *bg, int width, int height)
{
    xcb_free_pixmap(xorg.connection, bg->pixmap);
    bg->pixmap = xcb_generate_id(xorg.connection);
    int rh = (bg->image->height * height) / VH;
    xcb_create_pixmap(xorg.connection, xorg.screen->root_depth, bg->pixmap, xorg.window.id, width, rh);
    xcb_image_t *r = resize_image(bg->image, width, rh);
    bg->cur_height = rh;
    xcb_request_check(xorg.connection, 
        xcb_image_put(xorg.connection, 
                      bg->pixmap, 
                      xorg.window.gc, r, 0, 0, 0));
    free(r->data);
    free(r);
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

void render_image(v_window_t window, xcb_image_t *image, int dest_x, int dest_y)
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

v_window_t virtual_window_init(x11_t xorg, int w, int h)
{
    v_window_t result; 
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

void free_v_window(v_window_t v_window, xcb_connection_t *con)
{
    xcb_shm_detach(con, v_window.info.shmseg);
    shmdt(v_window.info.shmaddr);
    xcb_free_pixmap(con, v_window.pix);
}

background_t background_init(x11_t xorg)
{
    background_t bg = {
        .y = 0,
        .image = load_image("bg.png", 0, 0, xorg),
        .pixmap = xcb_generate_id(xorg.connection)
    };

    xcb_create_pixmap(xorg.connection, 
                      xorg.screen->root_depth, 
                      bg.pixmap, 
                      xorg.window.id, 
                      bg.image->width, bg.image->height);
    return bg;
}

xcb_image_t *load_image(char *file, int width, int height, x11_t xorg)
{
    int channels, size, w, h;
    unsigned char *data = stbi_load(file, &w, &h, &channels, STBI_rgb_alpha);
    for (int i = 0; i < w * h; i++) {
        unsigned char *pixel = data + (i * channels);
        char tmp = *pixel;
        *pixel  = pixel[2];
        pixel[2] = tmp;
    }

    if (!width || !height) 
        return xcb_image_create_native(xorg.connection, 
                                       w, h, 
                                       XCB_IMAGE_FORMAT_Z_PIXMAP,
                                       xorg.screen->root_depth, 
                                       data, w * h * 4, data);
    size = width * height * channels;
        
    unsigned char *resized_data = malloc(size);
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(data,
                 w, h, 0,
                 resized_data, width, height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);
    stbi_image_free(data);

    return xcb_image_create_native(xorg.connection, 
                                   width, height, 
                                   XCB_IMAGE_FORMAT_Z_PIXMAP, 
                                   xorg.screen->root_depth, 
                                   resized_data, size, resized_data);
}

xcb_image_t *resize_image(xcb_image_t *image, int width, int height)
{
    int size = width * height * 4;
    unsigned char *resized_data = malloc(size);
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(image->data,
                 image->width, image->height, 0,
                 resized_data, width, height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);

    xcb_image_t *image_out = malloc(sizeof(xcb_image_t));
    *image_out = *image;
    image_out->size = size;
    image_out->width = width;
    image_out->height = height;
    image_out->data = resized_data;

    return image_out;
}
