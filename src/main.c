#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include "global.h"
#include "states/states.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../libs/stb_image.h"
#include "../libs/stb_image_resize.h"

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
void window_free(xcb_connection_t *c, window_t window);
void bg_free(xcb_connection_t *c, background_t bg);

sound_element_t load_sound_file(char *file);
int unload_sound_file(sound_element_t sound);
int free_sound(sounds_t sound);

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
    const long time_per_frame = SECONDS / 60;
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
    free_font(xorg.font);
    free_sound(xorg.sounds);
    free(xorg.textures.data);
    bg_free(xorg.connection, xorg.bg);
    free_v_window(xorg.v_window, xorg.connection);
    window_free(xorg.connection, xorg.window);
    xcb_disconnect(xorg.connection);
    free(keyboard);
    return 0;
}

int free_sound(sounds_t sound) 
{
    for (int i = 0; i < SOUND_MAX; i++) 
        unload_sound_file(sound.sounds[i]);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(sound.context);
    alcCloseDevice(sound.device);
    return 0;
}

int unload_sound_file(sound_element_t sound)
{
    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);
    return 0;
}

void sound_pause(sounds_t sound, enum sounds_enum i) 
{
    return alSourcePause(sound.sounds[i].source);
}

char isSoundPlaying(sounds_t sound, enum sounds_enum i)
{
    int state; 
    alGetSourcei(sound.sounds[i].source, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING) ? 1: 0;
}

void sound_play(sounds_t sound, enum sounds_enum i)
{
    return alSourcePlay(sound.sounds[i].source);
}

sound_element_t load_sound_file(char *file)
{
    sound_element_t sound; 
    alGenSources((ALuint)1, &sound.source);

    alSourcef(sound.source, AL_PITCH, 1);
    alSourcef(sound.source, AL_GAIN, 1);
    alSource3f(sound.source, AL_POSITION, 0, 0, 0);
    alSource3f(sound.source, AL_VELOCITY, 0, 0, 0);
    alSourcei(sound.source, AL_VELOCITY, AL_FALSE);

    alGenBuffers((ALuint)1, &sound.buffer);

    ALsizei size, freq;
    ALenum format;
    ALvoid *data;
    ALboolean loop = AL_FALSE;

    alutLoadWAVFile(file, &format, &data, &size, &freq, &loop);
    alBufferData(sound.buffer, format, data, size, freq);

    alSourcei(sound.source, AL_BUFFER, sound.buffer);
    return sound;
}

sounds_t sound_init()
{
    sounds_t sound;
    sound.device = alcOpenDevice(NULL);
    if (!sound.device) 
    {
        perror("Error: opening device\n");
        exit(1);
    }

    sound.context = alcCreateContext(sound.device, NULL);
    if (!alcMakeContextCurrent(sound.context)) 
    {
        perror("Error: setting context current\n");
        exit(1);
    }

    sound.sounds[SOUND_LAUNCH]      = load_sound_file("resources/launch.wav");
    sound.sounds[SOUND_SHOOT]       = load_sound_file("resources/shoot.wav");
    sound.sounds[SOUND_BREAK]       = load_sound_file("resources/break.wav");
    sound.sounds[SOUND_GAME_OVER]   = load_sound_file("resources/game_over.wav");
    sound.sounds[SOUND_SELECT]      = load_sound_file("resources/select.wav");
    return sound;
}

image_t flip_image(image_t image)
{
    int *buffer = malloc(image.width*image.height*sizeof(int));
    for (int y = 0; y < image.height; y++) 
        for (int x = 0; x < image.width; x++) 
            buffer[(x + (y * image.width))] = 
                 ((int*)image.data)[(image.width - x) + 
                  (y * image.y_offset)];

    return (image_t) { 
        .data = (unsigned char*)buffer,
        .width = image.width, 
        .height = image.height,
        .y_offset = image.width 
    };
}

image_t slice_texture(image_t texture, int x, int y, int w, int h, char cpy)
{
    if (!cpy)
        return (image_t) { 
            .data = &texture.data[(x + (y * texture.width)) * 4],
            .width = w, 
            .height = h,
            .y_offset = texture.width
        };

    int *buffer = malloc(w*h*4);
    for (int dy = 0; dy < h; dy++) 
        for (int dx = 0; dx < w; dx++) 
            buffer[(dx + (dy * w))] = 
                 ((int*)texture.data)[(dx + x) + 
                  ((dy + y) * texture.y_offset)];
    return (image_t) { 
        .data = (unsigned char*)buffer,
        .width = w, 
        .height = h,
        .y_offset = w 
    };
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
    xorg.textures   = load_image("resources/textures.png", 0, 0, xorg);
    xorg.sounds     = sound_init();

    font_init("resources/font.ttf", &xorg.font);
    return xorg;
}

void window_free(xcb_connection_t *c, window_t window)
{
    xcb_free_gc(c, window.gc);
    xcb_unmap_window(c, window.id);
    xcb_destroy_window(c, window.id);
}


window_t window_init(x11_t xorg)
{
    window_t window = {
        .x = xorg.screen->width_in_millimeters / 2,  
        .y = 0,
        .width  = VW * 1.6, 
        .height = VH * 1.6, 
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

void free_font(font_t font)
{
    FT_Done_Face(font.face);
    FT_Done_FreeType(font.ft);
}

int font_init(char *name, font_t *font)
{
    FT_Init_FreeType(&font->ft);
    
    if (FT_New_Face(font->ft, name, 0, &font->face)) 
    {
        printf("error loading font");
        return 1; 
    }
    return 0;
}

int render_text(v_window_t window, font_t font, char *str, int dest_x, int dest_y, int color)
{
    FT_Set_Pixel_Sizes(font.face, translate_x(window,font.w), translate_y(window, font.h));
    int real_x = translate_x(window, dest_x);
    int real_y = translate_y(window, dest_y) + font.h;

    for (char *ptr = str; *ptr; ptr++) 
    {
        if (FT_Load_Char(font.face, *ptr, FT_LOAD_RENDER))
        {
            printf("*ptr = %c\n", *ptr);
            perror("Error: loading char\n");
            return 1;
        }

        
        FT_GlyphSlot g = font.face->glyph;
        char *bitmap = g->bitmap.buffer;

        int w = g->bitmap.width; 
        int h = g->bitmap.rows;


        for (int y = 0; y < h && y + real_y < window.h; y++) 
            for (int x = 0; x < w && x + real_x < window.w; x++) 
            {
                if (!bitmap[x + (y * w)])
                    continue;

                int ny = y + real_y - g->bitmap_top;
                if (ny < 0) 
                    continue;
                window.buffer[(x + real_x) + (ny * window.w)] = color;
            }

        real_x += g->advance.x >> 6;
    }

    return 0;
}

void bg_free(xcb_connection_t *c, background_t bg)
{
    xcb_free_pixmap(c, bg.pixmap);
    free(bg.image.data);
}

void resize_bg(x11_t xorg, background_t *bg, int width, int height)
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

void change_state(x11_t xorg, int state)
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


int translate_y(v_window_t window, int y)
{
    return (y * window.h) / VH;
}

int translate_x(v_window_t window, int x)
{
    return (x * window.w) / VW;
}

xcb_rectangle_t translate_rect_pos(v_window_t window, xcb_rectangle_t rect)
{
    return (xcb_rectangle_t) { 
        .x = translate_x(window, rect.x),
        .y = translate_y(window, rect.y),
        .width = translate_x(window, rect.width),
        .height = translate_y(window, rect.height)
    };
}

void render_image(v_window_t window, image_t image, int dest_x, int dest_y)
{
    int ndx = translate_x(window, dest_x);
    int ndy = translate_y(window, dest_y); 

    int xo = 0, yo = 0;
    if (dest_y < 0) 
        yo = ndy;
    if (dest_x < 0) 
        xo = ndx; 

    int nw = (image.width * window.w) / VW;
    int nh = (image.height * window.h) / VH;

    double x_scale = (double)image.width / nw; 
    double y_scale = (double)image.height / nh; 


    for (int y = ndy - yo; y < ndy + nh && y < window.h; y++) {
        for (int x = ndx - xo; x < ndx + nw && x < window.w; x++) {
           int src_x = (int) ((x - ndx) * x_scale);
           int src_y = (int) ((y - ndy) * y_scale);

            unsigned char *pixel = &image.data[(src_x + (src_y * image.y_offset)) * 4];
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
        .image = load_image("resources/bg.png", 0, 0, xorg),
        .pixmap = xcb_generate_id(xorg.connection)
    };

    xcb_create_pixmap(xorg.connection, 
                      xorg.screen->root_depth, 
                      bg.pixmap, 
                      xorg.window.id, 
                      bg.image.width, bg.image.height);
    return bg;
}

image_t load_image(char *file, int width, int height, x11_t xorg)
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
        return (image_t) {
            .data = data, 
            .width = w,
            .height = h, 
            .y_offset = w
        };

    size = width * height * channels;
        
    unsigned char *resized_data = malloc(size);
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(data,
                 w, h, 0,
                 resized_data, width, height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);
    stbi_image_free(data);

    return (image_t) { 
        .data = resized_data,
        .width = width,
        .height = height, 
        .y_offset = width 
    };
}

image_t resize_image(image_t image, int width, int height)
{
    int size = width * height * 4;
    unsigned char *resized_data = malloc(size);
    stbir_filter filter = STBIR_FILTER_BOX;
    stbir_resize(image.data,
                 image.width, image.height, 0,
                 resized_data, width, height, 0,
                 STBIR_TYPE_UINT8, 4, 1, 1, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 filter, filter, STBIR_COLORSPACE_LINEAR, NULL);

    return (image_t) { 
        .data = resized_data,
        .y_offset = width, 
        .width = width,
        .height = height, 
    };
}


