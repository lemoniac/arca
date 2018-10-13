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
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    if(keys.size() < 16)
                        keys.push_back(event.key.keysym.sym);
                    break;

                case SDL_MOUSEMOTION:
                    mousePos.x = event.motion.x;
                    mousePos.y = event.motion.y;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if(mouse.size() < 16)
                    {
                        MouseEvent me = {event.button.button, event.button.state, {event.button.x, event.button.y}};
                        mouse.push_back(me);
                    }
                    break;
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

    bool shortInstruction = baseaddr[PC] & SHORT_INSTRUCTION;

    if(shortInstruction)
        currentInst = *(uint16_t *)(baseaddr + PC);
    else
        currentInst = *(unsigned *)(baseaddr + PC);
    uint8_t opcode = currentInst & 0x7f;
    unsigned dst, src0, src1, imm, width;
    switch(opcode)
    {
        case MOVI:
            decodeC(dst, imm);
            regs[dst] = imm;
            break;

        case SHORT_MOVI:
            decodeShortB(dst, imm);
            regs[dst] = imm;
            break;

        case MOVR:
            decodeA(dst, src0, src1, imm);
            regs[dst] = regs[src0];
            break;

        case SHORT_MOVR:
            decodeShortA(dst, src0);
            regs[dst] = regs[src0];
            break;

        case LUI: {
            decodeC(dst, imm);
            regs[dst] = imm << 12;
        }

        case LOADR: {
            decodeD(dst, src0, width, imm);
            int off = extendSign(imm, 14);
            unsigned addr = regs[src0] + off;
            switch(width)
            {
                case MEM_LOADW: regs[dst] = *(unsigned *)(baseaddr + addr); break;
                case MEM_LOADHU: regs[dst] = *(uint16_t *)(baseaddr + addr); break;
                case MEM_LOADBU: regs[dst] = *(uint8_t *)(baseaddr + addr); break;
            }
            break;
        }

        case STORER: {
            decodeD(dst, src0, width, imm);
            int off = extendSign(imm, 14);
            unsigned addr = regs[dst] + off;
            unsigned value = regs[src0];
            switch(width)
            {
                case MEM_STOREW: *(unsigned *)(baseaddr + addr) = value; break;
                case MEM_STOREH: *(uint16_t *)(baseaddr + addr) = value; break;
                case MEM_STOREB: *(uint8_t *)(baseaddr + addr) = value; break;
            }
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

                case ALU_SLT: res = (regs[src0] < regs[src1]) ? 1 : 0; break;
            }

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SHORT_ALUR+ALU_ADD:
        case SHORT_ALUR+ALU_SUB:
        case SHORT_ALUR+ALU_MUL:
        case SHORT_ALUR+ALU_DIV:
        case SHORT_ALUR+ALU_SHR:
        case SHORT_ALUR+ALU_SHL:
        case SHORT_ALUR+ALU_AND:
        case SHORT_ALUR+ALU_OR:
        case SHORT_ALUR+ALU_XOR: {
            uint32_t res = 0;
            decodeShortA(dst, src0);
            switch(opcode - SHORT_ALUR)
            {
                case ALU_ADD: res = regs[dst] + regs[src0]; break;
                case ALU_SUB: res = regs[dst] - regs[src0]; break;
                case ALU_MUL: res = regs[dst] * regs[src0]; break;
                case ALU_DIV: res = regs[dst] / regs[src0]; break;
                case ALU_SHL: res = regs[dst] << regs[src0]; break;
                case ALU_SHR: res = regs[dst] >> regs[src0]; break;

                case ALU_AND: res = regs[dst] & regs[src0]; break;
                case ALU_OR:  res = regs[dst] | regs[src0]; break;
                case ALU_XOR: res = regs[dst] ^ regs[src0]; break;
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

        case SLTI: {
            decodeB(dst, src0, imm);
            regs[dst] = (regs[src0] < imm)? 1 : 0;
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

        case SHORT_CMPR: {
            decodeShortA(src0, src1);
            uint32_t res = regs[src0] - src1;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            break;
        }

        case SHORT_CMPI: {
            decodeShortB(src0, imm);
            uint32_t res = regs[src0] - imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            break;
        }

        case INT:
        {
            unsigned n, _;
            decodeC(n, _);
            interrupt(n);
            break;
        }

        case SHORT_INT:
        {
            unsigned n;
            decodeShortC(n);
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

        case SHORT_JMPR:
            decodeShortB(dst, imm);
            PC = regs[dst] - 2;
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

    PC += shortInstruction? 2 : 4;

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

void VM::decodeD(unsigned &dst, unsigned &src, unsigned &fun, unsigned &imm)
{
    dst = (currentInst >> 7) & 0x1f;
    src = (currentInst >> 12) & 0x1f;
    fun = (currentInst >> 17) & 0x7;
    imm = (currentInst >> 20);
}

void VM::decodeShortA(unsigned &dst, unsigned &src)
{
    dst = (currentInst >> 7) & 0xf;
    src = (currentInst >> 11);
}

void VM::decodeShortB(unsigned &dst, unsigned &imm)
{
    dst = (currentInst >> 7) & 0xf;
    imm = (currentInst >> 11);
}

void VM::decodeShortC(unsigned &imm)
{
    imm = (currentInst >> 7);
}

int VM::extendSign(unsigned imm, unsigned bit)
{
    if(imm & (1 << bit))
        return imm | 0xFFFFFFFF << bit;
    return imm;
}
