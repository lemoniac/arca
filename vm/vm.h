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

    const unsigned VRAM_BASEADDR = 0x10000000;
    const unsigned TEXT_BASEADDR = 0x20000000;
    const unsigned TEXT_BASEADDR_MAX = 0x20000000 + (80 * 30 * 2);

    void setGpuEnabled(bool enabled) { gpuEnabled = enabled; }

protected:

    void interrupt(uint8_t n);
    void enterKernelMode(unsigned entrypoint, unsigned exitpoint);

    unsigned currentInst;
    void decodeA(unsigned &dst, unsigned &src0, unsigned &src1, unsigned &imm);
    void decodeB(unsigned &dst, unsigned &src, unsigned &imm);
    void decodeC(unsigned &dst, unsigned &imm);
    void decodeD(unsigned &dst, unsigned &src, unsigned &fun, unsigned &imm);

    void decodeShortA(unsigned &dst, unsigned &src);
    void decodeShortB(unsigned &dst, unsigned &imm);
    void decodeShortC(unsigned &imm);

    int extendSign(unsigned imm, unsigned bit);

    file_system fs;

    struct MousePos {
        int x;
        int y;
    };

    struct MouseEvent {
        uint8_t button;
        uint8_t state;
        MousePos pos;
    };

    std::deque<char> keys;
    std::deque<MouseEvent> mouse;
    MousePos mousePos;

    bool interruptsEnabled = false;
    bool gpuEnabled = true;
};

#endif//ARCA__VM__H
