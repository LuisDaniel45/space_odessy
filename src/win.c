#include <stdio.h>
#include <windows.h>
#include <wingdi.h>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance     = hInst;
    wc.lpszClassName = L"window_class";
    wc.lpfnWndProc   = WindowProcedure;
    if (!RegisterClassW(&wc))
    {
        perror("Erorr: Registering class\n");
        return 1;
    }

    CreateWindowW(L"window_class", 
                  L"My Window", 
                  WS_OVERLAPPED | WS_VISIBLE, 
                  0, 0, 500, 500, 
                  NULL, NULL, NULL, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

#define SPEED 10
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static int x = 20, y = 10;
    switch (msg) {
        case WM_CHAR:
            switch (wp) {
                case 'h':
                    y -= SPEED;
                    break;

                case 'j':
                    x += SPEED;
                    break;

                case 'k':
                    x -= SPEED;
                    break;

                case 'l':
                    y += SPEED;
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
            InvalidateRgn(hwnd, NULL, TRUE);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT: {
            LPPAINTSTRUCT p;
            HDC hdc = BeginPaint(hwnd, p);

            HBRUSH blue = CreateSolidBrush(RGB(0, 0, 255)); 
            RECT rect = {
                .top = y,
                .left = x,
                .right = x + 50,
                .bottom = y + 50
            };
            FillRect(hdc, &rect, blue);
            DeleteObject(blue);

            EndPaint(hwnd, p);
        }


        default:
            return DefWindowProcW(hwnd, msg, wp, lp);

    
    }
}
