
#include <xcb/shm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "../global.h" 
#include "window.h"
#include <xcb/xcb.h>

void window_free(xcb_connection_t *c, window_t window)
{
    xcb_free_gc(c, window.gc);
    xcb_unmap_window(c, window.id);
    xcb_destroy_window(c, window.id);
}


window_t window_init(xcb_connection_t *c, xcb_screen_t *s)
{
    window_t window = {
        .x = s->width_in_millimeters / 2,  
        .y = 0,
        .width  = VW * 1.6, 
        .height = VH * 1.6, 
        .title  = "Space Odessy",
        .id     = xcb_generate_id(c),
        .gc     = xcb_generate_id(c),
    };


    // set window config
    int mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    int values[] = {0x00010114, XCB_EVENT_MASK_KEY_RELEASE | 
                                XCB_EVENT_MASK_KEY_PRESS   | 
                                XCB_EVENT_MASK_EXPOSURE    };

    // create window 
    xcb_create_window(c, s->root_depth, window.id, s->root,                
                      window.x, window.y,               
                      window.width, window.height,     
                      BORDERW,                         
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,    
                      s->root_visual,         
                      mask, values);                    

    mask = XCB_GC_FOREGROUND;
    int values_gc[] = {0x00000000};
    xcb_create_gc(c, window.gc, window.id, mask, &values_gc);

    // set window title
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, 
                        window.id, 
                        XCB_ATOM_WM_NAME, 
                        XCB_ATOM_STRING, 8, 
                        strlen(window.title), window.title);

    // map window to screen to make it visible 
    xcb_map_window(c, window.id);
    return window;
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

v_window_t virtual_window_init(xcb_connection_t *c, 
                               xcb_screen_t *s, 
                               xcb_window_t win, int w, int h)
{
    v_window_t result; 
    xcb_shm_query_version_reply_t*  reply;
    reply = xcb_shm_query_version_reply(c, xcb_shm_query_version(c), NULL);
    if(!reply || !reply->shared_pixmaps)
    {
        printf("Shm error...\n");
        exit(0);
    }

    result.info.shmid   = shmget(IPC_PRIVATE, w*h*4, IPC_CREAT | 0777);
    result.info.shmaddr = shmat(result.info.shmid, 0, 0);

    result.info.shmseg = xcb_generate_id(c);
    xcb_shm_attach(c, result.info.shmseg, result.info.shmid, 0);
    shmctl(result.info.shmid, IPC_RMID, 0);

    result.buffer = result.info.shmaddr;

    result.pix = xcb_generate_id(c);
    xcb_shm_create_pixmap(c, result.pix, win, 
                          w, h, s->root_depth, 
                          result.info.shmseg, 0);
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

