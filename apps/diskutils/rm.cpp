#include "filesystem.h"

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("rm image file\n");
        return 1;
    }

    file_system fs;
    fs.init(argv[1]);
    fs.remove_file(argv[2]);

    return 0;
}
