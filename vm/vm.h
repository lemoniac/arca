#ifndef ARCA__VM__H
#define ARCA__VM__H

#include <stdio.h>
#include "gpu.h"

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

    const unsigned SectorSize = 512;
    FILE *disk = NULL;

    ~VM();

    void setDisk(const char *filename);

    void init();

    void run();

protected:
    void interrupt(uint8_t n);
};

#endif//ARCA__VM__H
