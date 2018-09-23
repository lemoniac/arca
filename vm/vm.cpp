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
            regs[dst] = *(unsigned *)(code + addr);
            break;
        }

        case STORE: {
            uint8_t src = code[PC+1];
            uint16_t addr = *(uint16_t *)(code+PC+2);
            *(unsigned *)(code + addr) = regs[src];
            break;
        }

        case LOADR: {
            uint8_t dst = code[PC+1];
            uint16_t addr = regs[code[PC+2]];
            uint8_t off = code[PC+3];
            regs[dst] = *(unsigned *)(code + addr + off);
            break;
        }

        case STORER: {
            uint8_t addr = regs[code[PC+1]];
            uint8_t value = regs[code[PC+2]];
            uint8_t off = code[PC+3];
            code[addr + off] = value;
            break;
        }

        case ADD: {
            uint8_t dst = code[PC+1];
            uint8_t src0 = code[PC+2];
            uint8_t src1 = code[PC+3];
            uint32_t res = regs[src0] + regs[src1];

            is_zero = res == 0;
            sign = (res >> 31) == 1;

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
            sign = (res >> 31) == 1;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case AND: {
            uint8_t dst = code[PC+1];
            uint8_t src0 = code[PC+2];
            uint8_t src1 = code[PC+3];
            uint32_t res = regs[src0] & regs[src1];

            is_zero = res == 0;

            if (dst != 0)
                regs[dst] = res;

            break;
        }

        case INC: {
            uint8_t dst = code[PC+1];
            uint16_t imm = *(uint16_t *)(code+PC+2);
            uint32_t res = regs[dst] + imm;

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
                case COND_SIGN: jmp = sign; break;
                case COND_NOSIGN: jmp != sign; break;
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

        case JALR:
        {
            uint8_t dst = code[PC+1];
            uint8_t src = code[PC+2];
            regs[dst] = PC + 4;
            PC = regs[src] - 4;
            break;
        }

        default:
            std::cerr << "unknown opcode " << unsigned(code[PC]) << std::endl;
            return false;
    }

    PC += 4;

    return true;
}

