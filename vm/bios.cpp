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

        case 0x13:
            switch(regs[1])
            {
                case 0: // has disk
                    regs[1] = (disk == NULL)?0:1;
                    break;

                case 1: { // read sector
                    if(!disk)
                    {
                        regs[1] = 0;
                        return;
                    }

                    int err = fseek(disk, regs[2] * SectorSize, SEEK_SET);
                    size_t res = fread(data + regs[3], 1, SectorSize, disk);
                    regs[1] = 1;
                    break;
                }

                case 2: { // write sector
                    if(!disk)
                    {
                        regs[1] = 0;
                        return;
                    }

                    int err = fseek(disk, regs[2] * SectorSize, SEEK_SET);
                    size_t res = fwrite(data + regs[3], 1, SectorSize, disk);
                    regs[1] = 1;
                    break;
                }
            }
    }
}
