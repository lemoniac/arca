#ifndef ARCA__GPU__H
#define ARCA__GPU__H

#include <SDL2/SDL.h>

enum class Primitive : unsigned {
    Point = 0, Line = 1, Rect = 2, Triangle = 3
};

class GPU {
public:
    void init();
    void setForegroundColor(unsigned color);
    void drawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b);
    void drawLine(int x0, int y0, int x1, int y1);
    void drawRectangle(int x0, int y0, int x1, int y1);
    void setCursor(unsigned x, unsigned y);
    void drawChar(uint8_t c);
    void clearScreen(unsigned color);
    void draw(unsigned type, unsigned numVertex, int *vertices);
    void flush();

    void memory(unsigned address, unsigned value);
    unsigned memory(unsigned address);
    unsigned vram_size() const;

    void screenshot();

protected:
    SDL_Window *window;
    SDL_Surface *surface;

    uint8_t font[128*256];
    unsigned cursor_x = 0;
    unsigned cursor_y = 0;

    unsigned foreground_color;

    void drawLineLo(int x0, int y0, int x1, int y1);
    void drawLineHi(int x0, int y0, int x1, int y1);
};


#endif//ARCA__GPU__H
