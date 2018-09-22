#ifndef ARCA__OPCODES__H
#define ARCA__OPCODES__H

const unsigned MOVI = 1;
const unsigned MOVR = 2;

const unsigned LOAD = 3;
const unsigned STORE = 4;
const unsigned LOADR = 5;
const unsigned STORER = 6;

const unsigned ADD = 10;
const unsigned SUB = 11;
const unsigned MUL = 12;
const unsigned DIV = 13;
const unsigned SHR = 14;
const unsigned SHL = 15;
const unsigned INC = 16;

const unsigned JMP = 20;
const unsigned JMPR = 21;
const unsigned JAL = 22;
const unsigned JALR = 23;

const unsigned AND = 30;
const unsigned OR = 31;
const unsigned XOR = 32;
const unsigned NOT = 33;

const unsigned INT = 100;


const unsigned COND_ALLWAYS = 0;
const unsigned COND_ZERO = 1;
const unsigned COND_NOTZERO = 2;

#endif//ARCA__OPCODES__H
