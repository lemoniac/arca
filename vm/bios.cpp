#include <iostream>
#include "vm.h"
#include "opcodes.h"

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
                case 6: gpu.flush(); break;

                case 10: gpu.drawChar(regs[2]); break;
                case 11: gpu.setCursor(regs[2], regs[3]); break;
            }
            break;

        case 0x13:
            switch(regs[1])
            {
                case 0: // has disk
                    regs[1] = fs.connected()?1:0;
                    break;

                case 1: { // read sector
                    if(!fs.connected())
                    {
                        regs[1] = 0;
                        return;
                    }

                    fs.read_sector(regs[2], RAM + regs[3]);
                    regs[1] = 1;
                    break;
                }

                case 2: { // write sector
                    if(!fs.connected())
                    {
                        regs[1] = 0;
                        return;
                    }

                    fs.write_sector(regs[2], RAM + regs[3]);
                    regs[1] = 1;
                    break;
                }
            }

        case 0x16:
            switch(regs[1])
            {
                case 0:
                    if(keys.empty())
                        regs[1] = 0;
                    else
                    {
                        regs[1] = keys[0];
                        keys.pop_front();
                    }
                    break;
            }
            break;

        default: {
            if(interruptsEnabled)
            {
                unsigned *idt = (unsigned *)(RAM + CR[CR_IDT]);
                if(idt[n] != 0)
                {
                    regs[1] = n;
                    enterKernelMode(idt[n], PC);
                }
            }
        }
    }
}
