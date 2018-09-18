.data
    char str = "hello world!"
    int str_z = 0

.text
    jmp start

    .include ../libs/gfx.S

start:
    r2 = 255
    call clear_screen

    r2 = 10
    r3 = 5
    call set_cursor

    r2 = &str
    call puts
