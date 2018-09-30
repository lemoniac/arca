r14 = 4092

jmp start


accum:
    push r15

    r0 = r2 - r0
    jmp.z accum_exit

    r1 = r1 + r2
    r2 = r2 - 1
    call accum

accum_exit:
    pop r15
    ret


start:
    r1 = 0
    r2 = 10
    call accum

    r2 = r1
    r1 = 0
    int 0
