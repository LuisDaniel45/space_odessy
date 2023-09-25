#pragma once

typedef struct {
    void *face;
    void *ft;
    int w;
    int h;
} font_t;

void free_font(font_t font);
int font_init(char *name, font_t *font);
int render_text(v_window_t window, font_t font, char *str, int dest_x, int dest_y, int color);
