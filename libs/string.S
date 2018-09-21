# int strlen(const char *string)
strlen:
    r1 = 0
    r3 = r2

    strlen_loop:
        r2 = *r3

        r0 = r2 - r0
        jmp.z print_loop_exit

        r1 += 1
        r3 += 1
        jmp print_loop

    strlen_loop_exit:
    ret


strcat:
    ret

strcpy:
    ret

# r2: dst
# r3: value
# r4: size
memset:
    r0 = r4 - r0
    jmp.z memset_end

    r6 = 1

    memset_loop:
        r4 = r4 - r6
        *r2 = r3
        jmp.z memset_end

        r2 += 1

        jmp memset_loop

    memset_end:
    ret
