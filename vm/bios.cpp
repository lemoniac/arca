#include <iostream>
#include "vm.h"

void VM::interrupt(uint8_t n)
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
