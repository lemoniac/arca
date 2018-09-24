#ifndef ARCA__OPCODES__H
#define ARCA__OPCODES__H

#define MOVI 1
#define MOVR 2

#define LOAD   3
#define STORE  4
#define LOADR  5
#define STORER 6

#define ADD 10
#define SUB 11
#define MUL 12
#define DIV 13
#define SHR 14
#define SHL 15
#define INC 16

#define JMP  20
#define JMPR 21
#define JAL  22
#define JALR 23

#define AND  30
#define ANDI 31
#define OR   32
#define ORI  33
#define XOR  34
#define XORI 35
#define NOT  36

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
