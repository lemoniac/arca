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

void decodeShortA(unsigned inst, unsigned *dst, unsigned *src)
{
    *dst = (inst >> 7) & 0xf;
    *src = (inst >> 11);
}

void decodeShortB(unsigned inst, unsigned *dst, unsigned *imm)
{
    *dst = (inst >> 7) & 0xf;
    *imm = (inst >> 11);
}

void decodeShortC(unsigned inst, unsigned *imm)
{
    *imm = (inst >> 7);
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
        unsigned i = 0;
        while(i < bytes)
        {
            int short_encoding = buffer[i] & SHORT_INSTRUCTION;
            unsigned inst = short_encoding?  *(uint16_t *)(buffer + i) : *(unsigned *)(buffer + i);
            printf("%04u  ", i);
            uint8_t opcode = buffer[i] & 0x7f;
            switch(opcode)
            {
                case MOVI: {
                    decodeC(inst, &dst, &imm);
                    printf("r%u = %u\n", dst, imm);
                    break;
                }

                case SHORT_MOVI: {
                    decodeShortB(inst, &dst, &imm);
                    printf("r%u = %u\n", dst, imm);
                    break;
                }

                case MOVR: {
                    decodeA(inst, &dst, &src0, &src1, &imm);
                    printf("r%u = r%u\n", dst, src0);
                    break;
                }

                case SHORT_MOVR: {
                    decodeShortA(inst, &dst, &src0);
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

                case ALU: {
                    char *op = "?";
                    decodeA(inst, &dst, &src0, &src1, &imm);
                    switch(imm)
                    {
                        case ALU_ADD: op = "+"; break;
                        case ALU_SUB: op = "-"; break;
                        case ALU_MUL: op = "*"; break;
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

                case SHORT_ALUR+ALU_ADD:
                case SHORT_ALUR+ALU_SUB:
                case SHORT_ALUR+ALU_MUL:
                case SHORT_ALUR+ALU_DIV:
                case SHORT_ALUR+ALU_SHR:
                case SHORT_ALUR+ALU_SHL:
                case SHORT_ALUR+ALU_AND:
                case SHORT_ALUR+ALU_OR:
                case SHORT_ALUR+ALU_XOR: {
                    char *op = "?";
                    decodeShortA(inst, &dst, &src0);
                    uint8_t alu = opcode - SHORT_ALUR;
                    switch(alu)
                    {
                        case ALU_ADD: op = "+"; break;
                        case ALU_SUB: op = "-"; break;
                        case ALU_MUL: op = "*"; break;
                        case ALU_DIV: op = "/"; break;
                        case ALU_SHL: op = "<<"; break;
                        case ALU_SHR: op = ">>"; break;

                        case ALU_AND: op = "&"; break;
                        case ALU_OR:  op = "|"; break;
                        case ALU_XOR: op = "^"; break;
                    }
                    printf("r%u = r%u %s r%u\n", dst, dst, op, src0);
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

                case SHLI:
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u << %u\n", dst, src0, imm);
                    break;

                case SHRI:
                    decodeB(inst, &dst, &src0, &imm);
                    printf("r%u = r%u >> %u\n", dst, src0, imm);
                    break;

                case INCI: {
                    decodeC(inst, &dst, &imm);
                    printf("r%u += %u\n", dst, imm);
                    break;
                }

                case SHORT_CMPR:
                    decodeShortA(inst, &dst, &src0);
                    printf("r0 = r%u - r%u\n", dst, src0);
                    break;

                case SHORT_CMPI:
                    decodeShortB(inst, &src0, &imm);
                    printf("r0 = r%u - %i\n", src0, imm);
                    break;

                case INT:
                    decodeC(inst, &dst, &imm);
                    printf("int 0x%x\n", dst);
                    break;

                case SHORT_INT:
                    decodeShortC(inst, &imm);
                    printf("int 0x%x\n", imm);
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

                case SHORT_JMPR:
                    decodeShortB(inst, &dst, &imm);
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

            if(short_encoding == 0)
                i += 4;
            else
                i += 2;
        }
    }

    return 0;
}//