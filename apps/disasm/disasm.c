#include <stdio.h>
#include <stdint.h>
#include "opcodes.h"

int main(int argc, char **argv)
{
    FILE *file = fopen(argv[1], "rb");
    if(!file)
        return 1;

    unsigned char buffer[512];
    unsigned offset = 0;
    while(!feof(file))
    {
        unsigned bytes = fread(buffer, 1, 512, file);
        for(unsigned i = 0; i < bytes; i+= 4, offset+= 4)
        {
            printf("%04u  ", offset);
            switch(buffer[i])
            {
                case MOVI: {
                    uint8_t dst = buffer[i+1];
                    uint16_t imm = *(uint16_t *)(buffer+i+2);
                    printf("r%u = %u\n", dst, imm);
                    break;
                }

                case MOVR: {
                    uint8_t dst = buffer[i+1];
                    uint8_t src = buffer[i+2];
                    printf("r%u = r%u\n", dst, src);
                    break;
                }

                case LOAD: {
                    uint8_t dst = buffer[i+1];
                    uint16_t addr = *(uint16_t *)(buffer+i+2);
                    printf("r%u = [%u]\n", dst, addr);
                    break;
                }

                case LOADR: {
                    uint8_t dst = buffer[i+1];
                    uint8_t src = buffer[i+2];
                    uint8_t off = buffer[i+3];
                    printf("r%u = r%u[%u]\n", dst, src, off);
                    break;
                }

                case ADD: {
                    uint8_t dst = buffer[i+1];
                    uint8_t src0 = buffer[i+2];
                    uint8_t src1 = buffer[i+3];
                    printf("r%u = r%u + r%u\n", dst, src0, src1);
                    break;
                }

                case SUB: {
                    uint8_t dst = buffer[i+1];
                    uint8_t src0 = buffer[i+2];
                    uint8_t src1 = buffer[i+3];
                    printf("r%u = r%u - r%u\n", dst, src0, src1);
                    break;
                }

                case AND: {
                    uint8_t dst = buffer[i+1];
                    uint8_t src0 = buffer[i+2];
                    uint8_t src1 = buffer[i+3];
                    printf("r%u = r%u & r%u\n", dst, src0, src1);
                    break;
                }

                case INC: {
                    uint8_t dst = buffer[i+1];
                    uint16_t imm = *(uint16_t *)(buffer+i+2);
                    printf("r%u += %u\n", dst, imm);
                    break;
                }

                case INT:
                    printf("int 0x%x\n", buffer[i+1]);
                    break;

                case JMP: {
                    uint8_t cond = buffer[i+1];
                    uint16_t imm = *(uint16_t *)(buffer+i+2);
                    printf("jmp");
                    switch(cond)
                    {
                        case COND_ZERO: printf(".z"); break;
                        case COND_NOTZERO: printf(".nz"); break;
                    }

                    printf(" %u\n", imm);
                    break;
                }

                case JMPR:
                    printf("jmp r%u\n", buffer[i+1]);
                    break;

                case JAL: {
                    uint8_t dst = buffer[i+1];
                    uint16_t imm = *(uint16_t *)(buffer+i+2);
                    printf("jal r%u %u\n", dst, imm);
                    break;
                }

                default:
                    printf("??? %2u %2u %2u %2u\n", buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]);
            }
        }
    }

    return 0;
}