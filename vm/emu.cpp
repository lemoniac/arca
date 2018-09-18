#include <stdio.h>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include "opcodes.h"

class GPU {
public:
    void init()
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

    void drawPixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b)
    {
        if (x >= 640 || y >= 480) return;

        uint8_t *s = (uint8_t *)surface->pixels;
        unsigned offset = y * surface->pitch + x * 4;
        s[offset] = r;
        s[offset+1] = g;
        s[offset+2] = b;

        SDL_UpdateWindowSurface(window);     
    }

    void setCursor(unsigned x, unsigned y)
    {
        cursor_x = x * 8;
        cursor_y = y * 16;
    }

    void drawChar(uint8_t c)
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

    void clearScreen(unsigned color)
    {
        for(unsigned i = 0; i < 640 * 480; i++)
            ((unsigned *)surface->pixels)[i] = color;

        SDL_UpdateWindowSurface(window);
    }

protected:
    SDL_Window *window;
    SDL_Surface *surface;

    uint8_t font[128*256];
    unsigned cursor_x = 0;
    unsigned cursor_y = 0;

    unsigned foreground_color;
};

class VM {
public:
    unsigned PC = 0;
    unsigned DP = 0; // data pointer

    unsigned codesize = 0;
    unsigned datasize = 0;

    uint32_t regs[32] = {};

    uint8_t code[16 * 1024];
    uint8_t data[16 * 1024];

    bool is_zero;

    GPU gpu;

    void init()
    {
        gpu.init();
    }

    void run()
    {
        while(PC < codesize)
        {
            uint8_t opcode = code[PC];
            switch(opcode)
            {
                case MOVI: {
                    uint8_t dst = code[PC+1];
                    uint16_t imm = *(uint16_t *)(code+PC+2);
                    regs[dst] = imm;
                }
                break;

                case MOVR: {
                    uint8_t dst = code[PC+1];
                    uint8_t src = code[PC+2];
                    regs[dst] = regs[src];
                }
                break;

                case LOAD: {
                    uint8_t dst = code[PC+1];
                    uint16_t addr = *(uint16_t *)(code+PC+2);
                    regs[dst] = *(unsigned *)(data + addr);
                    break;
                }

                case STORE: {
                    uint8_t src = code[PC+1];
                    uint16_t addr = *(uint16_t *)(code+PC+2);
                    *(unsigned *)(data + addr) = regs[src];
                    break;
                }

                case LOADR: {
                    uint8_t dst = code[PC+1];
                    uint16_t addr = regs[code[PC+2]];
                    uint8_t off = code[PC+3];
                    regs[dst] = *(unsigned *)(data + addr + off);
                    break;
                }

                case STORER: {
                    uint8_t addr = regs[code[PC+1]];
                    uint8_t value = regs[code[PC+2]];
                    uint8_t off = code[PC+3];
                    data[addr + off] = value;
                    break;
                }

                case ADD: {
                    uint8_t dst = code[PC+1];
                    uint8_t src0 = code[PC+2];
                    uint8_t src1 = code[PC+3];
                    uint32_t res = regs[src0] + regs[src1];

                    is_zero = res == 0;

                    if (dst != 0)
                        regs[dst] = res;

                    break;
                }

                case SUB: {
                    uint8_t dst = code[PC+1];
                    uint8_t src0 = code[PC+2];
                    uint8_t src1 = code[PC+3];
                    uint32_t res = regs[src0] - regs[src1];

                    is_zero = res == 0;

                    if (dst != 0)
                        regs[dst] = res;

                    break;
                }

                case INT:
                    interrupt(code[PC+1]);
                    break;

                case JMP:
                {
                    uint8_t cond = code[PC+1];
                    uint16_t imm = *(uint16_t *)(code+PC+2);
                    bool jmp = false;
                    switch(cond)
                    {
                        case COND_ALLWAYS: jmp = true; break;
                        case COND_ZERO: jmp = is_zero; break;
                        case COND_NOTZERO: jmp = !is_zero; break;
                    }
                    if(jmp)
                        PC = imm - 4;

                    break;
                }

                case JMPR:
                    PC = regs[code[PC+1]] - 4;
                    break;

                case JAL:
                {
                    uint8_t dst = code[PC+1];
                    uint16_t imm = *(uint16_t *)(code+PC+2);
                    regs[dst] = PC + 4;
                    PC = imm - 4;
                    break;
                }

                default:
                    std::cerr << "unknown opcode " << unsigned(code[PC]) << std::endl;
                    break;
            }

            PC += 4;
        }

        for(int i = 0; i < 32; i++)
            std::cout << "r" << i << ": " << regs[i] << "    ";
        std::cout << std::endl;
    }

protected:
    void interrupt(uint8_t n)
    {
        switch(n)
        {
            case 0:
            {
                switch(regs[1])
                {
                    case 0:
                        std::cout << regs[2];
                        break;

                    case 1:
                    {
                        std::cout << char(regs[2]);
                        break;
                    }
                }
                break;
            }

            case 0x10:
                switch(regs[1])
                {
                    case 0:
                        gpu.drawPixel(regs[2], regs[3], regs[4] & 0xff, (regs[4] >> 8) & 0xff, (regs[4] >> 16) & 0xff);
                        break;

                    case 5: gpu.clearScreen(regs[2]); break;

                    case 10: gpu.drawChar(regs[2]); break;
                    case 11: gpu.setCursor(regs[2], regs[3]); break;
                }
                break;
        }
    }
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "emu file\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "rt");
    if(!file)
    {
        fprintf(stderr, "error loading file %s\n", argv[1]);
        return 1;
    }

    VM vm;

    fread(&vm.codesize,4, 1, file);
    fread(&vm.datasize,4, 1, file);

    fread(vm.code, vm.codesize, 1, file);
    fread(vm.data, vm.datasize, 1, file);

    vm.init();
    vm.run();

    SDL_Delay(5000);

    return 0;
}
