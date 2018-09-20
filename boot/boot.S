.data
    char err_no_filesystem = "no filesystem"
    int z0 = 0
    char err_no_kernel = "no kernel"
    int z1 = 0
    char loading_kernel = "loading kernel "
    int z2 = 0
    char ok_msg = "[OK]"
    int z3 = 0

sector:

.text

jmp main

.include ../libs/gfx.S

main:
    # check filesystem
    r1 = 0
    int 19

    r0 = r1 - r0
    r2 = &err_no_filesystem
    jmp.z error

    # read file table
    r1 = 1
    r2 = 0
    r3 = &sector
    int 19

    # check first file
    r1 = *r3
    r0 = r1 - r0
    r2 = &err_no_kernel

    jmp.z error

    r2 = &loading_kernel
    call puts

    # locate block
    r3 = r3 + 10
    r3 = *r3
    r4 = 65535
    r4 = r3 & r4

    # read block sector
    r1 = 1
    r2 = 1
    r3 = &sector
    int 19

    r3 = r3 + r4

    # copy kernel to memory
    r4 = 65536
    r10 = 4096
    r11 = 512

copy_kernel:
    r2 = *r3
    r2 = r2 & r4

    r0 = r2 - r0
    jmp.z copy_kernel_end

    r1 = 1
    r2 = r11
    r3 = r10
    int 19

    r3 += 2
    r10 += 512

    jmp copy_kernel

copy_kernel_end:

    r2 = &ok_msg
    call puts

    r10 = 4096
    jmp r10

error:
    call puts
