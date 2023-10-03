#include <stdio.h>
#undef linux
#define _WIN32_LEAN_AND_MEAN
#include "global.h"

#define SECONDS 1000
#define BG_SPEED 100

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

background_t background_init(HDC hdc);
void resize_bg(background_t *bg, int w, int h, HDC hdc);

void render_end(v_window_t window, HDC hdc, int w);
void render_begin(v_window_t window, background_t bg);

global_t g;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    g.window = window_init(WindowProcedure, hInst, L"Space Odessy", VW, VH);

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
            render_end(g.v_window, g.window.hdc, g.window.width);

            counter = 0;
            time = 0;
        }

        end = msg.time;
        time += end - start; 
        start = end;

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // update 
        counter++;
    }

    window_free(g.window);
    return 0;
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{

    switch (msg) {
        case WM_CHAR:
            if (wp == 'q')
            {
                PostQuitMessage(0);
            }
            break;


        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CREATE:
            g.textures = load_image("resources/textures.png", 0, 0);
            g.v_window = virtual_window_init(g.window.hdc, VW, VH);
            g.bg = background_init(g.window.hdc);
            g.sounds = sound_init();
            font_init("resources/font.ttf", &g.font);
            break;
            
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
                    }}, (HBRUSH)COLOR_WINDOW);
            break;

        default:
            return DefWindowProcW(hwnd, msg, wp, lp);

    
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
}

background_t background_init(HDC hdc)
{
    background_t bg = {
        .y = 0,
        .cur_height = VH,
        .image = load_image("resources/bg.png", 0, 0),
        .hdc = CreateCompatibleDC(hdc)
    };

    HBITMAP bitmap = CreateCompatibleBitmap(hdc, bg.image.width, bg.image.height); 
    SetBitmapBits(bitmap, VW*VH* 4, bg.image.data);
    SelectObject(bg.hdc, bitmap);
    DeleteObject(bitmap);
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
    BitBlt(window.hdc, 0, 0, 
           window.w, window.h, 
           bg.hdc, 0, 0, SRCCOPY);
}
