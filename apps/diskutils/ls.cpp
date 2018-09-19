#include <stdio.h>
#include "filesystem.h"

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("ls image\n");
        return 1;
    }

    file_system fs;
    fs.init(argv[1]);

    for(int i = 0; i < ENTRIES_PER_SECTOR; i++)
    {
        file_entry *entry = fs.entry(i);
        if(entry->filename[0] != 0)
            printf("%s %u\n", entry->filename, entry->size);
    }

    return 0;
}