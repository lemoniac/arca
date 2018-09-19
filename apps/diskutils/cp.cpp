#include <stdio.h>
#include "filesystem.h"

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("cp [image:]src [image:]dst\n");
        return 1;
    }

    return 0;
}
