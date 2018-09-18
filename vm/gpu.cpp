#include <iostream>
#include "gpu.h"

void GPU::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        std::cerr << "Error initialization SDL Video" << std::endl;
    window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    surface = SDL_GetWindowSurface(window);

    FILE *file = fopen("vga8x12_extra_chars.tga", "rb");
    if (!file)
    {
        std::cerr << "Couldn't open font file" << std::endl;
        return;
    }
    fseek(file, 18, SEEK_SET);
    fread(font, 1, 128*256, file);
    fclose(file);
}

void GPU::drawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= 640 || y >= 480) return;

    uint8_t *s = (uint8_t *)surface->pixels;
    unsigned offset = y * surface->pitch + x * 4;
    s[offset] = r;
    s[offset+1] = g;
    s[offset+2] = b;

    SDL_UpdateWindowSurface(window);     
}

void GPU::setCursor(unsigned x, unsigned y)
{
    cursor_x = x * 8;
    cursor_y = y * 16;
}

void GPU::drawChar(uint8_t c)
{
    unsigned offset_c = (15 - (c / 16)) * 128 * 16 + (c % 16) * 8;
    unsigned offset_s = 4 * cursor_x + cursor_y * surface->pitch;
    uint8_t *s = (uint8_t *)surface->pixels;

    for(unsigned y = 0; y < 16; y++)
        for(unsigned x = 0; x < 8; x++)
        {
            unsigned o = offset_s + (y * surface->pitch + 4 * x);
            uint8_t c = font[offset_c + (15 - y) * 128 + x];
            if(c != 0)
            {
                s[o] = c;
                s[o+1] = c;
                s[o+2] = c;
            }
        }

    SDL_UpdateWindowSurface(window);

    cursor_x += 8;
    if(cursor_x >= 640)
    {
        cursor_x = 0;
        cursor_y += 16;
    }
}

void GPU::clearScreen(unsigned color)
{
    for(unsigned i = 0; i < 640 * 480; i++)
        ((unsigned *)surface->pixels)[i] = color;

    SDL_UpdateWindowSurface(window);
}
