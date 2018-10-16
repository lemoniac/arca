#ifndef ARCA__OPCODES__H
#define ARCA__OPCODES__H

#define MOVI 1
#define MOVR 2
#define LUI  3

#define LOADR  6
#define STORER 7

#define ALU  10
#define ADDI 11
#define SUBI 12
#define MULI 13
#define DIVI 14
#define SHLI 15
#define SHRI 16
#define INCI 17
#define ANDI 18
#define XORI 19
#define ORI  20
#define SLTI  21

#define JMP  30
#define JMPR 31
#define JAL  32
#define JALR 33
#define BRANCH 34

#define INT 50
#define SYSTEM 51

// Short instruction opcodes
#define SHORT_INSTRUCTION 0x40
#define SHORT_MOVI SHORT_INSTRUCTION
#define SHORT_MOVR (SHORT_INSTRUCTION+1)

#define SHORT_ALUR (SHORT_INSTRUCTION+2)

#define SHORT_CMPI (SHORT_INSTRUCTION+20)
#define SHORT_CMPR (SHORT_INSTRUCTION+21)
#define SHORT_JMPR (SHORT_INSTRUCTION+25)
#define SHORT_INT  (SHORT_INSTRUCTION+30)

// ALU operations
#define ALU_ADD  0
#define ALU_SUB  1
#define ALU_MUL  2
#define ALU_DIV  3
#define ALU_SHR  4
#define ALU_SHL  5
#define ALU_AND  6
#define ALU_OR   7
#define ALU_XOR  8
#define ALU_SLT  9

#define MEM_LOADW  0
#define MEM_LOADHU 1
#define MEM_LOADHS 2
#define MEM_LOADBU 3
#define MEM_LOADBS 4

#define MEM_STOREW 0
#define MEM_STOREH 1
#define MEM_STOREB 2

#define COND_ALWAYS  0
#define COND_ZERO    1
#define COND_NOTZERO 2
#define COND_SIGN    3
#define COND_NOSIGN  4
#define COND_LT      5
#define COND_LTU     6
#define COND_GE      7
#define COND_GEU     8
#define COND_EQ      9
#define COND_NE     10

#define SYSTEM_CR_SET  0
#define SYSTEM_CR_READ 1
#define SYSTEM_CALL    2
#define SYSTEM_RETURN  3

#define CR_ENTRYPOINT 0
#define CR_RETURNADDR 1
#define CR_COUNTER   10
#define CR_TIME      11
#define CR_IDT       12
#define CR_BASEADDR  20

#endif//ARCA__OPCODES__H
