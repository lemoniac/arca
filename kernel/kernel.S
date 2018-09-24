.org 4096

jmp start
    char init_msg = "initialising kernel"
    int z0 = 0
    char entrypoint_msg = "entry point"
    int z1 = 0

    .include ../libs/gfx.S

start:
    r1 = &entrypoint
    cr0 = r1

    r2 = &init_msg
    call puts

    syscall

loop:
    jmp loop


entrypoint:
