# int isdigit(int c)
isdigit:
    r0 = r1 - '0'
    jmp.s isdigit_false

    r0 = r1 - 58
    jmp.ns isdigit_false

    r1 = 1
    ret

isdigit_false:
    r1 = 0
    ret


# int islower(int c)
islower:
    r0 = r1 - 'a'
    jmp.s islower_false

    r0 = r1 - 123
    jmp.ns islower_false

    r1 = 1
    ret

islower_false:
    r1 = 0
    ret


# int isupper(int c)
isupper:
    r0 = r1 - 'A'
    jmp.s isupper_false

    r0 = r1 - 91
    jmp.ns isupper_false

    r1 = 1
    ret

isupper_false:
    r1 = 0
    ret
