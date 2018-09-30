#include <stdio.h>
#include <stdint.h>
#include "opcodes.h"

void decodeA(unsigned inst, unsigned *dst, unsigned *src0, unsigned *src1, unsigned *imm)
{
    *dst = (inst >> 7) & 0x1f;
    *src0 = (inst >> 12) & 0x1f;
    *src1 = (inst >> 17) & 0x1f;
    *imm = inst >> 22;
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

void decodeD(unsigned inst, unsigned *dst, unsigned *src, unsigned *fun, unsigned *imm)
{
    *dst = (inst >> 7) & 0x1f;
    *src = (inst >> 12) & 0x1f;
    *fun = (inst >> 17) & 0x7;
    *imm = (inst >> 20);
}

int extendSign(unsigned imm, unsigned bit)
{
    if(imm & (1 << bit))
        return imm | 0xFFFFFFFF << bit;
    return imm;
}

int main(int argc, char **argv)
{
    FILE *file = fopen(argv[1], "rb");
    if(!file)
        return 1;

    unsigned char buffer[512];
    unsigned offset = 0;
    unsigned dst, src0, src1, imm, width;

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
                    decodeA(inst, &dst, &src0, &src1, &imm);
                    printf("r%u = r%u\n", dst, src0);
                    break;
                }

                case LOADR: {
                    decodeD(inst, &dst, &src0, &width, &imm);
                    int imm_i = extendSign(imm, 14);
                    printf("r%u = r%u[%i]\n", dst, src0, imm_i);
                    break;
                }

                case STORER: {
                    decodeD(inst, &dst, &src0, &width, &imm);
                    int imm_i = extendSign(imm, 14);
                    char sgn = imm_i < 0? ' ' : '+';
                    printf("*r%u%c%i = r%u\n", dst, sgn, imm_i, src0);
                    break;
                }

                case ALU: {;
                    char *op = "?";
                    decodeA(inst, &dst, &src0, &src1, &imm);
                    switch(imm)
                    {
                        case ALU_ADD: op = "+"; break;
                        case ALU_SUB: op = "-"; break;
                        case ALU_MUL: op = "+"; break;
                        case ALU_DIV: op = "/"; break;
                        case ALU_SHL: op = "<<"; break;
                        case ALU_SHR: op = ">>"; break;

                        case ALU_AND: op = "&"; break;
                        case ALU_OR:  op = "|"; break;
                        case ALU_XOR: op = "^"; break;
                    }
                    printf("r%u = r%u %s r%u\n", dst, src0, op, src1);
                    break;
                }

                case ADDI: {
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u + %u\n", dst, src0, imm);
                    break;
                }

                case SUBI: {
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u - %u\n", dst, src0, imm);
                    break;
                }

                case INCI: {
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
                        case COND_ALLWAYS: break;
                        case COND_ZERO: printf(".z"); break;
                        case COND_NOTZERO: printf(".nz"); break;
                        case COND_SIGN: printf(".s"); break;
                        case COND_NOSIGN: printf(".ns"); break;
                        default: printf(".??");
                    }

                    printf(" %u\n", imm << 1);
                    break;
                }

                case JMPR:
                    decodeC(inst, &dst, &imm);
                    printf("jmp r%u\n", dst);
                    break;

                case JAL: {
                    decodeC(inst, &dst, &imm);
                    printf("jal r%u %u\n", dst, (imm << 1));
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