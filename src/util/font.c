#define STB_TRUETYPE_IMPLEMENTATION
#include "../../libs/stb_truetype.h"

#include "../global.h"
#include "font.h"

void error(char *s);
int read_file(char *file_name, unsigned char **file_content);

void free_font(font_t font)
{
    free(font);
}

int font_init(char *name, font_t *font)
{
    *font = malloc(sizeof(stbtt_fontinfo));
    unsigned char *file_content;
    int size = read_file(name, &file_content);
    stbtt_InitFont(*font, file_content, stbtt_GetFontOffsetForIndex(file_content, 0));
    return 0;
}

void render_text(v_window_t window, font_t font, char *str, int dest_x, int dest_y, int size, int color)
{
    dest_x = translate_x(window, dest_x);
    dest_y = translate_y(window, dest_y);
    size   = translate_y(window, size);

    float scale = stbtt_ScaleForPixelHeight(font, size);

    int w, h;
    for (char *ptr = str; *ptr; ptr++) {
        if (*ptr == ' ') 
        {
            dest_x += w; 
            continue;
        }

        int xoff, yoff, advance, lsb;
        unsigned char *bitmap = stbtt_GetCodepointBitmap(font, 0, scale, *ptr, &w,&h, &xoff,&yoff);
        stbtt_GetCodepointHMetrics(font, *ptr, &advance, &lsb);

        int dey = dest_y + size; 
        for (int y = 0; y < h; y++) 
            for (int x = 0;x < w;x++) 
                if (bitmap[x + (y * w)]) 
                    window.buffer[(x + dest_x) + ((y+dey+yoff) * window.w)] = color;
        dest_x += (advance * scale);
        free(bitmap);
    }
}

int read_file(char *file_name, unsigned char **file_content)
{
    FILE *file = fopen(file_name, "rb");
    if (!file) 
        error("error opening file");

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *file_content = malloc(file_size);
    if (fread(*file_content, 1, file_size, file) != file_size) {
        free(*file_content);
        error("Error: reading file");
    }

    fclose(file);
    return file_size;
}

void error(char *s)
{
    perror(s);
    exit(1);
}
