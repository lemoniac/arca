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

    uint8_t opcode = baseaddr[PC];
    switch(opcode)
    {
        case MOVI: {
            uint8_t dst = baseaddr[PC+1];
            uint16_t imm = *(uint16_t *)(baseaddr+PC+2);
            regs[dst] = imm;
        }
        break;

        case MOVR: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            regs[dst] = regs[src];
        }
        break;

        case LOAD: {
            uint8_t dst = baseaddr[PC+1];
            uint16_t addr = *(uint16_t *)(baseaddr+PC+2);
            regs[dst] = *(unsigned *)(baseaddr + addr);
            break;
        }

        case STORE: {
            uint8_t src = baseaddr[PC+1];
            uint16_t addr = *(uint16_t *)(baseaddr+PC+2);
            *(unsigned *)(baseaddr + addr) = regs[src];
            break;
        }

        case LOADR: {
            uint8_t dst = baseaddr[PC+1];
            uint16_t addr = regs[baseaddr[PC+2]];
            uint8_t off = baseaddr[PC+3];
            regs[dst] = *(unsigned *)(baseaddr + addr + off);
            break;
        }

        case STORER: {
            uint8_t addr = regs[baseaddr[PC+1]];
            uint8_t value = regs[baseaddr[PC+2]];
            uint8_t off = baseaddr[PC+3];
            baseaddr[addr + off] = value;
            break;
        }

        case ADD: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src0 = baseaddr[PC+2];
            uint8_t src1 = baseaddr[PC+3];
            uint32_t res = regs[src0] + regs[src1];

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SUB: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src0 = baseaddr[PC+2];
            uint8_t src1 = baseaddr[PC+3];
            uint32_t res = regs[src0] - regs[src1];

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SUBI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] - imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case MUL: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src0 = baseaddr[PC+2];
            uint8_t src1 = baseaddr[PC+3];
            uint32_t res = regs[src0] * regs[src1];

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case MULI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] * imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case DIV: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src0 = baseaddr[PC+2];
            uint8_t src1 = baseaddr[PC+3];
            uint32_t res = regs[src0] / regs[src1];

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case DIVI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] / imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SHRI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] >> imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case SHLI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] << imm;

            is_zero = res == 0;
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case AND: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src0 = baseaddr[PC+2];
            uint8_t src1 = baseaddr[PC+3];
            uint32_t res = regs[src0] & regs[src1];

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case ANDI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] & imm;

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case OR: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src0 = baseaddr[PC+2];
            uint8_t src1 = baseaddr[PC+3];
            uint32_t res = regs[src0] | regs[src1];

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case ORI: {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            uint8_t imm = baseaddr[PC+3];
            uint32_t res = regs[src] | imm;

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case INC: {
            uint8_t dst = baseaddr[PC+1];
            uint16_t imm = *(uint16_t *)(baseaddr+PC+2);
            uint32_t res = regs[dst] + imm;

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case INT:
            interrupt(baseaddr[PC+1]);
            break;

        case JMP:
        {
            uint8_t cond = baseaddr[PC+1];
            uint16_t imm = *(uint16_t *)(baseaddr+PC+2);
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
                PC = imm - 4;

            break;
        }

        case JMPR:
            PC = regs[baseaddr[PC+1]] - 4;
            break;

        case JAL:
        {
            uint8_t dst = baseaddr[PC+1];
            uint16_t imm = *(uint16_t *)(baseaddr+PC+2);
            regs[dst] = PC + 4;
            PC = imm - 4;
            break;
        }

        case JALR:
        {
            uint8_t dst = baseaddr[PC+1];
            uint8_t src = baseaddr[PC+2];
            regs[dst] = PC + 4;
            PC = regs[src] - 4;
            break;
        }

        case SYSTEM: {
            uint8_t fun = baseaddr[PC+1];
            switch(fun)
            {
                case SYSTEM_CR_SET: {
                    if(privilegeLevel == 0)
                    {
                        std::cerr << "General Protection Fault" << std::endl;
                        return false;
                    }
                    uint8_t src = baseaddr[PC+2];
                    uint8_t dst = baseaddr[PC+3];
                    CR[dst] = regs[src];
                    break;
                }

                case SYSTEM_CR_READ: {
                    if(privilegeLevel == 0)
                    {
                        std::cerr << "General Protection Fault" << std::endl;
                        return false;
                    }
                    uint8_t src = baseaddr[PC+2];
                    uint8_t dst = baseaddr[PC+3];
                    regs[src] = CR[dst];
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
