#include "../global.h" 
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../../libs/stb_image.h"
#include "../../libs/stb_image_resize.h"

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

image_t load_image(char *file, int width, int height)
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
