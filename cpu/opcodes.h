#ifndef ARCA__OPCODES__H
#define ARCA__OPCODES__H

#define MOVI 1
#define MOVR 2

#define LOAD   3
#define STORE  4
#define LOADR  5
#define STORER 6

#define ADD  10
#define ADDI 11
#define SUB  12
#define SUBI 13
#define MUL  14
#define DIV  15
#define SHR  16
#define SHL  17
#define INC  18

#define JMP  30
#define JMPR 31
#define JAL  32
#define JALR 33

#define AND  40
#define ANDI 41
#define OR   42
#define ORI  43
#define XOR  44
#define XORI 45
#define NOT  46

#define INT 100
#define SYSTEM 101

#define COND_ALLWAYS 0
#define COND_ZERO    1
#define COND_NOTZERO 2
#define COND_SIGN    3
#define COND_NOSIGN  4
#define COND_LT      5
#define COND_LTU     6
#define COND_GE      7
#define COND_GEU     8

#define SYSTEM_CR_SET  0
#define SYSTEM_CR_READ 1
#define SYSTEM_CALL    2
#define SYSTEM_RETURN  3

#define CR_ENTRYPOINT 0
#define CR_RETURNADDR 1
#define CR_COUNTER   10
#define CR_TIME      11
#define CR_IDT       12

#endif//ARCA__OPCODES__H
