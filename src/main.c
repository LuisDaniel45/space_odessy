#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "global.h"
#include "states/states.h"

global_t global_init();
void  free_global(global_t xorg);

background_t background_init(global_t xorg);
void resize_bg(global_t xorg, background_t *bg, int width, int height);
void bg_free(xcb_connection_t *c, background_t bg);

int *map_keyboard(global_t xorg, int *KeyDown[]);
void render_begin(global_t xorg);
void render_end(global_t xorg);

int main(int argc, char *argv[])
{
    global_t xorg = global_init();

    int *KeyDown[KEY_MAX];
    int *keyboard = map_keyboard(xorg, KeyDown);

    state_machine[cur_state].load(xorg);

    double time;
    int counter = 0;
    double dt = 100.0 / SECONDS;
    const int max_updates = 1; 
    const long time_per_frame = SECONDS / 60;
    struct timeval start, end;
    gettimeofday(&start, NULL);

    xcb_generic_event_t *event;
    while (1) 
    {
        if (counter > max_updates) 
        {
            dt = time / SECONDS;
            if (time_per_frame > time) 
                usleep(time_per_frame - time);

            render_begin(xorg);
            state_machine[cur_state].render(xorg);
            render_end(xorg);

            counter = 0;
            time = 0;
        }

        gettimeofday(&end, NULL);

        time += (((end.tv_sec - start.tv_sec) * SECONDS) +
                 (end.tv_usec - start.tv_usec));

        xorg.time += (int)time;

        start = end;

        **KeyDown = 0;
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
                    xorg.v_window = virtual_window_init(xorg.connection, 
                                                        xorg.screen, 
                                                        xorg.window.id, 
                                                        rw, expose->height);

                    xcb_clear_area(xorg.connection, 
                                   0, xorg.window.id, 
                                   0, 0, 
                                   expose->width, expose->height);
                    continue;
                }

                case XCB_KEY_PRESS: {
                    xcb_key_press_event_t *keycode = event;
                    keypress = (short) keyboard[keycode->detail];
                    *keyboard = keycode->detail;
                    if (keypress == KEY_quit) 
                        goto exit; 
                    key_pressed(keyboard, keycode->detail);
                    break;

                }

                case XCB_KEY_RELEASE: {
                    xcb_key_press_event_t *keycode = event;
                    key_release(keyboard, keycode->detail);
                    break;
                }
            }
        }

        state_machine[cur_state].update(&xorg, dt, KeyDown, keypress);
        xorg.bg.y = (xorg.bg.y + (int)(BG_SPEED * dt)) % xorg.bg.cur_height;
        counter++; 
    }

exit:
    printf("exit succesfully\n");
    free_global(xorg);
    free(keyboard);
    return 0;
}

void free_global(global_t xorg)
{
    free_font(xorg.font);
    sound_free(xorg.sounds);
    free(xorg.textures.data);
    bg_free(xorg.connection, xorg.bg);
    free_v_window(xorg.v_window, xorg.connection);
    window_free(xorg.connection, xorg.window);
    xcb_disconnect(xorg.connection);
}

global_t global_init()
{
    global_t xorg; 
    xorg.score      = 0;
    xorg.connection = xcb_connect(NULL, NULL);
    xorg.setup      = xcb_get_setup(xorg.connection);
    xorg.screen     = xcb_setup_roots_iterator(xorg.setup).data;

    xorg.window     = window_init(xorg.connection, 
                                  xorg.screen, 
                                  VW * 1.6, VH * 1.6, 
                                  "Space Odessy"); 

    xorg.v_window   = virtual_window_init(xorg.connection, 
                                          xorg.screen, 
                                          xorg.window.id, VW, VH);
    xorg.bg         = background_init(xorg);
    xorg.textures   = load_image("resources/textures.png", 0, 0);

    sound_init(xorg.sounds);
    font_init("resources/font.ttf", &xorg.font);

    xcb_flush(xorg.connection);
    return xorg;
}

int *map_keyboard(global_t xorg, int *KeyDown[])
{
    xcb_get_keyboard_mapping_reply_t* keyboard_mapping = 
        xcb_get_keyboard_mapping_reply(xorg.connection,
            xcb_get_keyboard_mapping(xorg.connection,
                xorg.setup->min_keycode,
                xorg.setup->max_keycode - xorg.setup->min_keycode + 1),
            NULL);

    xcb_keysym_t* keysyms  = (xcb_keysym_t*)(keyboard_mapping + 1);  
                                                                             
    int *keyboard = malloc(sizeof(int) * xorg.setup->max_keycode);
    *KeyDown = keyboard;
    for (int i = xorg.setup->min_keycode; i < xorg.setup->max_keycode; i++) {
        int keysym = keysyms[0 + (i - xorg.setup->min_keycode) * keyboard_mapping->keysyms_per_keycode];
        keyboard[i] = KEY_unmap;
        for (int j = 1; j < KEY_MAX; j++) 
            if (keysym == keys_table[j]) {
                keyboard[i] = j;
                KeyDown[j] = &keyboard[i];
                break;
            }
    }

    free(keyboard_mapping);
    return keyboard;
}     

void change_state(global_t xorg, int state)
{
    if (state_machine[cur_state].self) 
        free(state_machine[cur_state].self);
    
    cur_state = state;
    if (cur_state >= number_of_states) 
    {
        perror("Error: State doesn't exist");
        exit(1);
    }

    state_machine[cur_state].load(xorg);
} 

background_t background_init(global_t xorg)
{
    background_t bg = {
        .y = 0,
        .image = load_image("resources/bg.png", 0, 0),
        .pixmap = xcb_generate_id(xorg.connection)
    };

    xcb_create_pixmap(xorg.connection, 
                      xorg.screen->root_depth, 
                      bg.pixmap, 
                      xorg.window.id, 
                      bg.image.width, bg.image.height);
    return bg;
}

void bg_free(xcb_connection_t *c, background_t bg)
{
    xcb_free_pixmap(c, bg.pixmap);
    free(bg.image.data);
}

void resize_bg(global_t xorg, background_t *bg, int width, int height)
{
    xcb_free_pixmap(xorg.connection, bg->pixmap);
    bg->pixmap = xcb_generate_id(xorg.connection);
    int rh = (bg->image.height * height) / VH;
    xcb_create_pixmap(xorg.connection, xorg.screen->root_depth, bg->pixmap, xorg.window.id, width, rh);
    image_t r = resize_image(bg->image, width, rh);
    xcb_image_t *tmp = xcb_image_create_native(xorg.connection, r.width, r.height, XCB_IMAGE_FORMAT_Z_PIXMAP, xorg.screen->root_depth, r.data, r.width * r.height * 4, r.data);
    bg->cur_height = rh;
    xcb_request_check(xorg.connection, 
        xcb_image_put(xorg.connection, 
                      bg->pixmap, 
                      xorg.window.gc, tmp, 0, 0, 0));
    xcb_image_destroy(tmp);
}

void render_begin(global_t xorg) 
{
    int h = xorg.v_window.h;
    if (xorg.bg.y < xorg.v_window.h)
    {
        h = xorg.bg.y;
        xcb_copy_area(xorg.connection,
                      xorg.bg.pixmap,
                      xorg.v_window.pix,
                      xorg.window.gc,
                      0, 0, 0, xorg.bg.y,
                      xorg.v_window.w,
                      xorg.v_window.h - xorg.bg.y);
    }
    xcb_request_check(xorg.connection, 
            xcb_copy_area(
                xorg.connection,
                xorg.bg.pixmap,
                xorg.v_window.pix,
                xorg.window.gc, 0, 
                xorg.bg.cur_height - xorg.bg.y, 
                0, 0, xorg.v_window.w, h
            )
    );
}

void render_end(global_t xorg)
{
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
}

void color_free(color_t color, void *arg)
{
    xcb_free_gc(((global_t*)arg)->connection, color);
    return;
}

color_t create_color(int color, void *arg)
{
    global_t g = *(global_t*)arg;
    color_t out = xcb_generate_id(g.connection);
    xcb_create_gc(g.connection, out, 
                  g.window.id, XCB_GC_FOREGROUND, 
                  (int[]) {color});
    return out;
}

void render_rectangle(global_t g, rectangle_t rect, color_t color)
{
    xcb_rectangle_t scale_pos = translate_rect_pos(g.v_window, rect);
    xcb_poly_fill_rectangle(g.connection, g.v_window.pix, color, 1, &scale_pos);
}

