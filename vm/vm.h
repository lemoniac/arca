#ifndef ARCA__VM__H
#define ARCA__VM__H

#include <deque>
#include "filesystem.h"
#include "gpu.h"

class VM {
public:
    unsigned PC = 0;
    unsigned DP = 0; // data pointer

    unsigned codesize = 0;
    unsigned datasize = 0;

    uint32_t regs[32] = {};

    uint8_t RAM[64 * 1024];
    //uint8_t data[16 * 1024];

    bool is_zero = false;
    bool sign = false;

    unsigned privilegeLevel = 1;

    // control registers
    unsigned CR[256] = {};

    GPU gpu;

    ~VM();

    void setDisk(const char *filename);

    void init();

    void run();

    bool step();

protected:

    void interrupt(uint8_t n);
    void enterKernelMode(unsigned entrypoint, unsigned exitpoint);

    file_system fs;

    std::deque<char> keys;

    bool interruptsEnabled = false;
};

#endif//ARCA__VM__H
