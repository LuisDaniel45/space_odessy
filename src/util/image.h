#pragma once

typedef struct { 
    int width;
    int height;
    int y_offset;
    unsigned char *data;
} image_t;

void    render_image(v_window_t window, image_t image, int dest_x, int dest_y);
image_t load_image(char *file, int width, int height);
image_t resize_image(image_t image, int width, int height);
image_t slice_texture(image_t texture, int x, int y, int w, int h, char cpy);
image_t flip_image(image_t image);
