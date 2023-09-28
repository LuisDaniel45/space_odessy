#pragma once

typedef void* font_t;

void free_font(font_t font);
int font_init(char *name, font_t *font);
void render_text(v_window_t window, font_t font, char *str, int dest_x, int dest_y, int size, int color);
