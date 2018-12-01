## Overview

RISC 32 bits load/store architecture

## Registers

There are 32 32bit registers, the last 16 can't be used in the short versions of the instructions.

r0 - always 0
r1-r13 - user mode registers
r14 - stack register
r15 - link register


### Control and Status Registers

CR_ENTRYPOINT 0 - entry point of the kernel
CR_RETURNADDR 1 - where to return when we return from the kernel
CR_COUNTER   10 - instruction counter
CR_TIME      11
CR_IDT       12
CR_BASEADDR  20 - base address of the process


## Instructions

### Arithmetic and Logic

*rd* the destination register

*rs, rt* the source registers

rd = rs *op* rt

where *op* is an arithmetic operator: +, -, *, / or a logical one: &, |, >> , <<

rd = !rs

rd += imm


### Jump

jmp nnn - jumps to the address
jmp.cond nnn -- jumps to the address if the condition is true

z - if the zero flag is set
nz - if the zero flag is not set
s - if the sign flag is set
ns - if the sign flag is not set


## Assembler
### Aliases

call address
  jal r15 address - sets the link register to PC and jumps to address


ret
  jmp r15 - restores the PC to the address stored in the link register


