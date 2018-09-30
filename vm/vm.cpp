#include <iostream>
#include "opcodes.h"
#include "vm.h"
#include "filesystem.h"

VM::~VM()
{
    fs.finalize();
}

void VM::setDisk(const char *filename)
{
    fs.init(filename);
}

void VM::init()
{
    gpu.init();
}

void VM::run()
{
    unsigned t0 = SDL_GetTicks();
    SDL_Event event;
    while(PC < 65536)
    {
        if(SDL_PollEvent(&event))
        {
            if(event.type == SDL_KEYDOWN)
            {
                if(keys.size() < 16)
                    keys.push_back(event.key.keysym.sym);
            }
        }

        unsigned t = SDL_GetTicks();
        if(t - t0 >= 10)
        {
            t0 = t;
            interrupt(1);
        }

        if(!step())
            break;
    }

    gpu.flush();

    for(int i = 0; i < 32; i++)
        std::cout << "r" << i << ": " << regs[i] << "    ";
    std::cout << std::endl;
}

bool VM::step()
{
    uint8_t *baseaddr = RAM;
    if(privilegeLevel == 0)
        baseaddr+= CR[CR_BASEADDR];


    currentInst = *(unsigned *)(baseaddr + PC);
    uint8_t opcode = currentInst & 0x7f;
    unsigned dst, src0, src1, imm;
    switch(opcode)
    {
        case MOVI: {
            decodeC(dst, imm);
            regs[dst] = imm;
        }
        break;

        case MOVR: {
            decodeA(dst, src0, src1, imm);
            regs[dst] = regs[src0];
        }
        break;

        case LUI: {
            decodeC(dst, imm);
            regs[dst] = imm << 12;
        }

        case LOADR: {
            decodeB(dst, src0, imm);
            unsigned addr = regs[src0];
            regs[dst] = *(unsigned *)(baseaddr + addr + imm);
            break;
        }

        case STORER: {
            decodeB(dst, src0, imm);
            unsigned addr = regs[dst];
            uint8_t value = regs[src0];
            baseaddr[addr + imm] = value;
            break;
        }

        case ALU: {
            uint32_t res = 0;
            decodeA(dst, src0, src1, imm);
            switch(imm)
            {
                case ALU_ADD: res = regs[src0] + regs[src1]; break;
                case ALU_SUB: res = regs[src0] - regs[src1]; break;
                case ALU_MUL: res = regs[src0] * regs[src1]; break;
                case ALU_DIV: res = regs[src0] / regs[src1]; break;
                case ALU_SHL: res = regs[src0] << regs[src1]; break;
                case ALU_SHR: res = regs[src0] >> regs[src1]; break;

                case ALU_AND: res = regs[src0] & regs[src1]; break;
                case ALU_OR:  res = regs[src0] | regs[src1]; break;
                case ALU_XOR: res = regs[src0] ^ regs[src1]; break;
            }

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case ADDI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] + imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SUBI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] - imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case MULI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] * imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case DIVI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] / imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SHRI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] >> imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SHLI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] << imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case ANDI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] & imm;

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case ORI: {
            decodeB(dst, src0, imm);
            uint32_t res = regs[src0] | imm;

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case INCI: {
            decodeC(dst, imm);
            uint32_t res = regs[dst] + imm;

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }
        case INT:
        {
            unsigned n, _;
            decodeC(n, _);
            interrupt(n);
            break;
        }

        case JMP:
        {
            unsigned cond;
            unsigned imm;
            decodeC(cond, imm);
            bool jmp = false;
            switch(cond)
            {
                case COND_ALLWAYS: jmp = true; break;
                case COND_ZERO: jmp = is_zero; break;
                case COND_NOTZERO: jmp = !is_zero; break;
                case COND_SIGN: jmp = sign; break;
                case COND_NOSIGN: jmp = !sign; break;
            }
            if(jmp)
                PC = (imm << 1) - 4;

            break;
        }

        case JMPR:
            decodeC(src0, imm);
            PC = regs[src0] - 4;
            break;

        case JAL:
            decodeC(dst, imm);
            regs[dst] = PC + 4;
            PC = (imm << 1) - 4;
            break;

        case JALR:
            decodeB(dst, src0, imm);
            regs[dst] = PC + 4;
            PC = regs[src0] - 4 + imm;
            break;

        case SYSTEM: {
            unsigned fun;
            decodeA(fun, dst, src0, imm);
            switch(fun)
            {
                case SYSTEM_CR_SET: {
                    if(privilegeLevel == 0)
                    {
                        std::cerr << "General Protection Fault" << std::endl;
                        return false;
                    }
                    CR[dst] = regs[src0];
                    break;
                }

                case SYSTEM_CR_READ: {
                    if(privilegeLevel == 0)
                    {
                        std::cerr << "General Protection Fault" << std::endl;
                        return false;
                    }
                    regs[src0] = CR[dst];
                    break;
                }

                case SYSTEM_CALL: enterKernelMode(CR[CR_ENTRYPOINT] - 4, PC + 4); break;

                case SYSTEM_RETURN: {
                    if(privilegeLevel == 0)
                    {
                        std::cerr << "General Protection Fault" << std::endl;
                        return false;
                    }

                    PC = CR[CR_RETURNADDR] - 4;
                    privilegeLevel = 0;
                    break;
                }
            }
            break;
        }

        default:
            std::cerr << "unknown opcode " << unsigned(baseaddr[PC]) << std::endl;
            return false;
    }

    PC += 4;

    CR[CR_COUNTER]++;

    return true;
}

void VM::enterKernelMode(unsigned entrypoint, unsigned exitpoint)
{
    CR[CR_RETURNADDR] = exitpoint;
    PC = entrypoint;
    privilegeLevel = 1;
}


void VM::decodeA(unsigned &dst, unsigned &src0, unsigned &src1, unsigned &imm)
{
    dst = (currentInst >> 7) & 0x1f;
    src0 = (currentInst >> 12) & 0x1f;
    src1 = (currentInst >> 17) & 0x1f;
    imm = currentInst >> 22;
}

void VM::decodeB(unsigned &dst, unsigned &src, unsigned &imm)
{
    dst = (currentInst >> 7) & 0x1f;
    src = (currentInst >> 12) & 0x1f;
    imm = (currentInst >> 17);
}

void VM::decodeC(unsigned &dst, unsigned &imm)
{
    dst = (currentInst >> 7) & 0x1f;
    imm = (currentInst >> 12);
}
