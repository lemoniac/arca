# clear_screen
#    r2 = color
clear_screen:
    r1 = 5
    int 16
    ret


# set_cursor
#    r2 = x
#    r3 = y
set_cursor:
    r1 = 11
    int 16
    ret


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


# set_pixel
#    r2 = x
#    r3 = y
#    r4 = color
set_pixel:
    r1 = 0
    int 16
    ret


gfx_flush:
    r1 = 6
    int 16
    ret
