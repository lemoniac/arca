.struct point_s
    int x
    int y
.endstruct

jmp start

struct point_s point

start:
    r1 = 111
    r2 = 222
    r10 = &point
    *r10.point_s.x = r1
    *r10.point_s.y = r2

    r1 = 0
    r2 = *r10
    int 0

    r10 += 4
    r2 = *r10
    int 0
