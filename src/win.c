#include <stdio.h>
#undef linux
#define _WIN32_LEAN_AND_MEAN

#include "global.h"
#include "states/states.h"

#define SECONDS 1000
#define BG_SPEED 100

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

background_t background_init(HDC hdc);
void resize_bg(background_t *bg, int w, int h, HDC hdc);

void render_end(v_window_t window, HDC hdc, int w);
void render_begin(v_window_t window, background_t bg);

global_t g;
int keyboard[255];
int keypress;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    int *key_down[KEY_MAX];
    for (int i = 0; i < KEY_MAX; i++)
    {
        key_down[i] = &keyboard[keys_table[i]];
        keyboard[keys_table[i]] = i;
    }

    g.window = window_init(WindowProcedure, hInst, L"Space Odessy", VW * 1.6, VH * 1.6);

    state_machine[cur_state].load(g);

    int counter = 0;
    long time = 0;
    double dt = 100.0 / SECONDS;
    const int max_updates = 1;
    const long time_per_frame = SECONDS / 60;
    long start = 0, end = 0;
    SetTimer(g.window.id, 1, 0, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (counter > max_updates)
        {
            dt = (double)time / SECONDS;

            if (time_per_frame > time)
                Sleep(time_per_frame - time);

            render_begin(g.v_window, g.bg);
            state_machine[cur_state].render(g);
            render_end(g.v_window, g.window.hdc, g.window.width);

            counter = 0;
            time = 0;
        }

        end = msg.time;
        time += end - start;
        start = end;

        keypress = 0;
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        state_machine[cur_state].update(g, dt, key_down, keypress);
        g.bg.y = (g.bg.y + (int)(BG_SPEED * dt)) % g.bg.cur_height;
        counter++;
    }

    window_free(g.window);
    return 0;
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
        case WM_KEYDOWN:
            if (wp == 'Q')
            {
                PostQuitMessage(0);
            }
            key_pressed(keyboard, wp);
            keypress = (short) (keyboard[wp]); 
            break;

        case WM_KEYUP:
            key_release(keyboard, wp);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CREATE: {
            g.window.hdc = GetDC(hwnd);
            g.bg = background_init(g.window.hdc);
            g.textures = load_image("resources\\textures.png", 0, 0);
            g.sounds = sound_init();
            font_init("resources\\font.ttf", &g.font);
            break;
        }

        case WM_SIZE:
            g.window.width = LOWORD(lp); 
            g.window.height = HIWORD(lp); 

            int rw = (int) ((float) (g.window.height * VW) / VH);
            resize_bg(&g.bg, rw, g.window.height, g.window.hdc);
            free_v_window(g.v_window);
            g.v_window = virtual_window_init(g.window.hdc, rw, g.window.height);

            FillRect(g.window.hdc, (RECT[]) {{
                        .top = 0,
                        .left = 0,
                        .right = g.window.width,
                        .bottom = g.window.height
                    }}, g.window.class.hbrBackground);
            break;

        default:
            return DefWindowProcW(hwnd, msg, wp, lp);
            break;
    }
}

void resize_bg(background_t *bg, int w, int h, HDC hdc)
{
    int rh = (bg->image.height * h) / VH;
    bg->cur_height = rh;
    image_t image = resize_image(bg->image, w, rh);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, w, rh); 
    SetBitmapBits(bitmap, w*rh*4, image.data);
    SelectObject(bg->hdc, bitmap);
    DeleteObject(bitmap);
    free(image.data);
}

background_t background_init(HDC hdc)
{
    background_t bg = {
        .y = 0,
        .image = load_image("resources\\bg.png", 0, 0),
        .hdc = CreateCompatibleDC(hdc)
    };
    bg.cur_height = bg.image.height;
    return bg;
}

void render_end(v_window_t window, HDC hdc, int w) 
{
    BitBlt(hdc, (w/2) - (window.w/2), 0,
           window.w, window.h,
           window.hdc, 0, 0, SRCCOPY);
}

void render_begin(v_window_t window, background_t bg)
{
    int h = window.h;
    if (bg.y < window.h)
    {
        h = bg.y;
        BitBlt(window.hdc, 0, bg.y,
               window.w, window.h - bg.y,
               bg.hdc, 0, 0, SRCCOPY);
    }

    BitBlt(window.hdc, 0, 0,
           window.w, h,
           bg.hdc, 0, bg.cur_height - bg.y, SRCCOPY);
}

void color_free(color_t color, void *arg)
{
    DeleteObject(color);
}

color_t create_color(int color, void *arg)
{
    char *ptr = &color;
    char tmp = *ptr;
    *ptr = ptr[2];
    ptr[2] = tmp; 
    return CreateSolidBrush(color);
}

void render_rectangle(global_t g, rectangle_t rect, color_t color)
{
    RECT rectangle = translate_rect_pos(g.v_window, rect);
    FillRect(g.v_window.hdc, &rectangle, color);
}

void change_state(global_t g, int state)
{
    if (state_machine[cur_state].self) 
        free(state_machine[cur_state].self);
    
    cur_state = state;
    if (cur_state >= number_of_states) 
    {
        perror("Error: State doesn't exist");
        exit(1);
    }

    state_machine[cur_state].load(g);
} 
