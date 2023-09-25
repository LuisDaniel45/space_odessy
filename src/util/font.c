#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include "../global.h"
#include "font.h"

void free_font(font_t font)
{
    FT_Done_Face(font.face);
    FT_Done_FreeType(font.ft);
}

int font_init(char *name, font_t *font)
{
    FT_Init_FreeType(&font->ft);
    
    if (FT_New_Face(font->ft, name, 0, &font->face)) 
    {
        printf("error loading font");
        return 1; 
    }
    return 0;
}

int render_text(v_window_t window, font_t font, char *str, int dest_x, int dest_y, int color)
{
    FT_Set_Pixel_Sizes(font.face, translate_x(window,font.w), translate_y(window, font.h));
    int real_x = translate_x(window, dest_x);
    int real_y = translate_y(window, dest_y) + font.h;

    for (char *ptr = str; *ptr; ptr++) 
    {
        if (FT_Load_Char(font.face, *ptr, FT_LOAD_RENDER))
        {
            printf("*ptr = %c\n", *ptr);
            perror("Error: loading char\n");
            return 1;
        }

        
        FT_GlyphSlot g = ((FT_Face) font.face)->glyph;
        char *bitmap = g->bitmap.buffer;

        int w = g->bitmap.width; 
        int h = g->bitmap.rows;


        for (int y = 0; y < h && y + real_y < window.h; y++) 
            for (int x = 0; x < w && x + real_x < window.w; x++) 
            {
                if (!bitmap[x + (y * w)])
                    continue;

                int ny = y + real_y - g->bitmap_top;
                if (ny < 0) 
                    continue;
                window.buffer[(x + real_x) + (ny * window.w)] = color;
            }

        real_x += g->advance.x >> 6;
    }

    return 0;
}
