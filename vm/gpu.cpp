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

void GPU::setForegroundColor(unsigned foregroundColor)
{
    foreground_color = foregroundColor;
}

void GPU::drawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= 640 || y >= 480) return;

    uint8_t *s = (uint8_t *)surface->pixels;
    unsigned offset = y * surface->pitch + x * 4;
    s[offset] = r;
    s[offset+1] = g;
    s[offset+2] = b;
}

void GPU::drawLineLo(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) { yi = -1; dy = -dy; }
    int D = 2 * dy - dx;
    int y = y0;

    uint8_t *s = (uint8_t *)surface->pixels;

    for (int x = x0; x <= x1; x++)
    {
        unsigned offset = y * surface->pitch + x * 4;
        *(unsigned *)(s + offset) = foreground_color;
        if (D > 0)
        {
            y = y + yi;
            D = D - 2 * dx;
        }
        D = D + 2 * dy;
    }
}

void GPU::drawLineHi(int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0) { xi = -1; dx = -dx; }
    int D = 2 * dy - dx;
    int x = x0;

    uint8_t *s = (uint8_t *)surface->pixels;

    for (int y = y0; y <= y1; y++)
    {
        unsigned offset = y * surface->pitch + x * 4;
        *(unsigned *)(s + offset) = foreground_color;
        if (D > 0)
        {
            x = x + xi;
            D = D - 2 * dy;
        }
        D = D + 2 * dx;
    }
}

void GPU::drawLine(int x0, int y0, int x1, int y1)
{
    if (abs(y1 - y0) < abs(x1 - x0))
    {
        if (x0 > x1)
            drawLineLo(x1, y1, x0, y0);
        else
            drawLineLo(x0, y0, x1, y1);
    }
    else
    {
        if (y0 > y1)
            drawLineHi(x1, y1, x0, y0);
        else
            drawLineHi(x0, y0, x1, y1);
    }
}

void GPU::drawRectangle(int x0, int y0, int x1, int y1)
{
    if(x0 > x1) std::swap(x0, x1);
    if(y0 > y1) std::swap(y0, y1);
    
    for(unsigned y = y0; y <= y1; y++)
    for(unsigned x = x0; x <= x1; x++)
    {
        uint8_t *s = (uint8_t *)surface->pixels;
        unsigned offset = y * surface->pitch + x * 4;
        *(unsigned *)(s + offset) = foreground_color;
    }
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
        if(cursor_y < 480 - 16)
            cursor_y += 16;
    }
}

void GPU::clearScreen(unsigned color)
{
    for(unsigned i = 0; i < 640 * 480; i++)
        ((unsigned *)surface->pixels)[i] = color;

    SDL_UpdateWindowSurface(window);
}

void GPU::draw(unsigned type, unsigned numVertex, int *vertices)
{
    uint8_t r = foreground_color;
    uint8_t g = foreground_color >> 8;
    uint8_t b = foreground_color >> 16;

    switch(Primitive(type))
    {
        case Primitive::Point:
            for(unsigned i = 0; i < numVertex; i++)
                drawPixel(vertices[2 * i], vertices[2 * i + 1], r, g, b);
            break;

        case Primitive::Line:
            for(unsigned i = 0; i < numVertex - 1; i++)
                drawLine(vertices[2 * i], vertices[2 * i + 1], vertices[2 * i + 2], vertices[2 * i + 3]);
            break;

        case Primitive::Rect:
            for(unsigned i = 0; i < numVertex - 1; i++)
                drawRectangle(vertices[2 * i], vertices[2 * i + 1], vertices[2 * i + 2], vertices[2 * i + 3]);
            break;
    }
}

void GPU::flush()
{
    SDL_UpdateWindowSurface(window);
}

void GPU::memory(unsigned address, unsigned value)
{
    ((unsigned *)surface->pixels)[address/4] = value;
}

unsigned GPU::memory(unsigned address)
{
    return ((unsigned *)surface->pixels)[address/4];
}

unsigned GPU::vram_size() const
{
    return 640 * 480 * 4;
}