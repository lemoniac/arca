.text
# clear_screen
#    r2 = color
clear_screen:
    r1 = 5
    int 16
    jmp r15


# set_cursor
#    r2 = x
#    r3 = y
set_cursor:
    r1 = 11
    int 16
    jmp r15


# puts
#    r2 = string
puts:
    r1 = 10
    r3 = r2

    print_loop:
        r2 = *r3

        r0 = r2 - r0
        jmp.z print_loop_exit

        int 16

        r3 += 1
        jmp print_loop

    print_loop_exit:

    ret
