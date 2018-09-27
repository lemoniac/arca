#include <stdio.h>
#include <stdint.h>
#include "opcodes.h"

void decodeA(unsigned inst, unsigned *dst, unsigned *src0, unsigned *src1)
{
    *dst = (inst >> 7) & 0x1f;
    *src0 = (inst >> 12) & 0x1f;
    *src1 = (inst >> 17) & 0x1f;
}

void decodeB(unsigned inst, unsigned *dst, unsigned *src, unsigned *imm)
{
    *dst = (inst >> 7) & 0x1f;
    *src = (inst >> 12) & 0x1f;
    *imm = (inst >> 17);
}

void decodeC(unsigned inst, unsigned *dst, unsigned *imm)
{
    *dst = (inst >> 7) & 0x1f;
    *imm = (inst >> 12);
}

int main(int argc, char **argv)
{
    FILE *file = fopen(argv[1], "rb");
    if(!file)
        return 1;

    unsigned char buffer[512];
    unsigned offset = 0;
    unsigned dst, src0, src1, imm;

    while(!feof(file))
    {
        unsigned bytes = fread(buffer, 1, 512, file);
        for(unsigned i = 0; i < bytes; i+= 4, offset+= 4)
        {
            unsigned inst = *(unsigned *)(buffer + i);
            printf("%04u  ", offset);
            switch(buffer[i] & 0x7f)
            {
                case MOVI: {
                    decodeC(inst, &dst, &imm);
                    printf("r%u = %u\n", dst, imm);
                    break;
                }

                case MOVR: {
                    decodeA(inst, &dst, &src0, &src1);
                    printf("r%u = r%u\n", dst, src0);
                    break;
                }

                case LOAD: {
                    uint8_t dst = buffer[i+1];
                    uint16_t addr = *(uint16_t *)(buffer+i+2);
                    printf("r%u = [%u]\n", dst, addr);
                    break;
                }

                case LOADR: {
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u[%u]\n", dst, src0, imm);
                    break;
                }

                case ADD: {
                    decodeA(inst, &dst, &src0, &src1);
                    printf("r%u = r%u + r%u\n", dst, src0, src1);
                    break;
                }

                case ADDI: {
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u + %u\n", dst, src0, imm);
                    break;
                }

                case SUB: {
                    decodeA(inst, &dst, &src0, &src1);
                    printf("r%u = r%u - r%u\n", dst, src0, src1);
                    break;
                }

                case SUBI: {
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u - %u\n", dst, src0, imm);
                    break;
                }

                case AND: {
                    decodeA(inst, &dst, &src0, &src1);
                    printf("r%u = r%u & r%u\n", dst, src0, src1);
                    break;
                }

                case INC: {
                    decodeC(inst, &dst, &imm);
                    printf("r%u += %u\n", dst, imm);
                    break;
                }

                case INT:
                    decodeC(inst, &dst, &imm);
                    printf("int 0x%x\n", dst);
                    break;

                case JMP: {
                    decodeC(inst, &dst, &imm);
                    printf("jmp");
                    switch(dst)
                    {
                        case COND_ZERO: printf(".z"); break;
                        case COND_NOTZERO: printf(".nz"); break;
                        case COND_SIGN: printf(".s"); break;
                        case COND_NOSIGN: printf(".ns"); break;
                        default: printf(".??");
                    }

                    printf(" %u\n", imm);
                    break;
                }

                case JMPR:
                    decodeC(inst, &dst, &imm);
                    printf("jmp r%u\n", dst);
                    break;

                case JAL: {
                    decodeC(inst, &dst, &imm);
                    printf("jal r%u %u\n", dst, imm);
                    break;
                }

                case SYSTEM: {
                    uint8_t fun = buffer[i+1];
                    if(fun == SYSTEM_CALL)
                        printf("syscall\n");
                    else
                        printf("SYSTEM %u\n", fun);
                    break;
                }

                default:
                    printf("??? %2u %2u %2u %2u\n", buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]);
            }
        }
    }

    return 0;
}