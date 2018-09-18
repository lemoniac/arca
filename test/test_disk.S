.data
    char ok = "done"
    int ok_z = 0
    char err = "error: disk not found"
    int err_z = 0

    char str = "test123"
    int std_z = 0

.text
    jmp main

    .include ../libs/gfx.S

main:

    r1 = 0
    int 19

    r2 = &err

    r0 = r1 - r0
    jmp.z print

    r1 = 2
    r2 = 0
    r3 = &str
    int 19

    r2 = 1
    int 19

    r0 = r1 - r0
    jmp.z print

    r2 = &ok

print:
    call puts
