#include <stdio.h>
#undef linux
#define _WIN32_LEAN_AND_MEAN
#include "global.h"

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

background_t background_init(HDC hdc);
void resize_bg(background_t *bg, int w, int h, HDC hdc);

void render_end(v_window_t window, HDC hdc, int w);
void render_begin(v_window_t window, background_t bg);
window_t window; 

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    window = window_init(WindowProcedure, hInst, "Space Odessy", VW, VH);

    SetTimer(window.id, 1, 0, NULL);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    window_free(window);
    return 0;
}

#define SPEED 10
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static int x = 20, y = 10;
    static image_t textures;
    static image_t player;
    static v_window_t v_window;
    static background_t bg;
    static font_t font;
    static sounds_t sound;

    switch (msg) {
        case WM_CHAR:
            switch (wp) {
                case 'h':
                    x -= SPEED;
                    break;

                case 'j':
                    y += SPEED;
                    break;

                case 'k':
                    y -= SPEED;
                    break;

                case 'l':
                    x += SPEED;
                    break;

                case 'p':
                    break;

                case 'q':
                    PostQuitMessage(0);
                    break;

                case VK_RETURN:
                    break;

                default:
                    printf("%i\n", VK_RETURN);
                    break;

            }
            render_begin(v_window, bg);
            render_image(v_window, player, x, y);
            render_end(v_window, window.hdc, window.width);
            break;


        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CREATE:
            textures = load_image("resources/textures.png", 0, 0);
            player = slice_texture(textures, 0, 0, 50, 50, 0); 
            v_window = virtual_window_init(window.hdc, VW, VH);
            bg = background_init(window.hdc);
            sound = sound_init();
            font_init("resources/font.ttf", &font);
            break;
            
        case WM_SIZE:
            window.width = LOWORD(lp); 
            window.height = HIWORD(lp); 

            int rw = (int) ((float) (window.height * VW) / VH);
            resize_bg(&bg, rw, window.height, window.hdc);
            free_v_window(v_window);
            v_window = virtual_window_init(window.hdc, rw, window.height);

            FillRect(window.hdc, (RECT[]) {{
                        .top = 0,
                        .left = 0,
                        .right = window.width,
                        .bottom = window.height 
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
