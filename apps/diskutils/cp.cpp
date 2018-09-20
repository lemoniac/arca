#include <stdio.h>
#include "filesystem.h"

// TODO: for now the file is copied from the host to the filesystem using the same name
//     it should be able to copy from filesystem to host and between filesystems, and
//     change the name of the file

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("cp src image\n");
        return 1;
    }

    file_system fs;
    fs.init(argv[2]);

    FILE *src = fopen(argv[1], "rb");
    if(!src)
    {
        fprintf(stderr, "error: couldn't open src file\n");
        return 1;
    }

    file_t dst = fs.open(argv[1]);

    while(!feof(src))
    {
        uint8_t buffer[SECTOR_SIZE];
        unsigned bytes = fread(buffer, 1, SECTOR_SIZE, src);
        dst.write(bytes, buffer);
    }

    fs.finalize();

    return 0;
}
