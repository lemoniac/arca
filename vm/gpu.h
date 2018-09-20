#ifndef ARCA__GPU__H
#define ARCA__GPU__H

#include <SDL2/SDL.h>

class GPU {
public:
    void init();
    void drawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b);
    void setCursor(unsigned x, unsigned y);
    void drawChar(uint8_t c);
    void clearScreen(unsigned color);
    void flush();

protected:
    SDL_Window *window;
    SDL_Surface *surface;

    uint8_t font[128*256];
    unsigned cursor_x = 0;
    unsigned cursor_y = 0;

    unsigned foreground_color;
};


#endif//ARCA__GPU__H
