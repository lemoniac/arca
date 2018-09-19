#include <string.h>
#include "filesystem.h"

int file_system::init(const char *filesystem)
{
    file = fopen(filesystem, "r+b");

    read_sector(0, (uint8_t *)entries);
    read_sector(1, (uint8_t *)blocks);

    memset(sectors, 0, 65536 / SECTOR_SIZE);

    for(unsigned i = 0; i < ENTRIES_PER_SECTOR; i++)
    {
        for(unsigned j = 0; j < ENTRIES_PER_BLOCK; j++)
            if(blocks[i].sector[i] != 0)
                sectors[blocks[i].sector[i]] = 1;
    }

    sectors[0] = sectors[1] = 1;
}

void file_system::finalize()
{
    fclose(file);
}

int file_system::create_file(const char *filename)
{
    int n = empty_entry();
    if(n < 0)
        return -1;

    file_entry *entry = entries + n;

    strncpy(entry->filename, filename, MAX_FILENAME_SIZE);

    int b = empty_block();
    blocks[b].next_block = 0xFFFF; // mark it as used
    entry->first_block = b;

    write_sector(0, (uint8_t *)entries);
    write_sector(1, (uint8_t *)blocks);

    return 0;
}

int file_system::remove_file(const char *filename)
{
    int n = find_file(filename);

    uint16_t b = entries[n].first_block;
    if(b != 0)
    {
        // free the sectors used
        for(unsigned i = 0; i < ENTRIES_PER_BLOCK; i++)
            if(blocks[b].sector[i] != 0)
                sectors[blocks[b].sector[i]] = 0;

        memset(&blocks[b], 0, sizeof(block_t));
        write_sector(1, (uint8_t *)blocks);
    }

    memset(&entries[n], 0, sizeof(file_entry));
    write_sector(0, (uint8_t *)entries);

    return 0;
}

int file_system::empty_entry()
{
    for(int i = 0; i < ENTRIES_PER_SECTOR; i++)
    {
        if(entries[i].filename[0] == 0)
            return i;
    }

    return -1;
}

int file_system::empty_block()
{
    for(int i = 0; i < ENTRIES_PER_SECTOR; i++)
    {
        if(blocks[i].next_block == 0)
            return i;
    }

    return -1;
}

file_entry *file_system::entry(unsigned n)
{
    if(n >= ENTRIES_PER_SECTOR)
        return NULL;

    return entries + n;
}

int file_system::find_file(const char *name)
{
    for(int i = 0; i < ENTRIES_PER_SECTOR; i++)
    {
        if(!strncmp(entries[i].filename, name, FILENAME_MAX))
            return i;
    }

    return -1;
}

int file_system::read_sector(unsigned n, uint8_t *sector)
{
    fseek(file, SECTOR_SIZE * n, SEEK_SET);
    fread(sector, 1, SECTOR_SIZE, file);

    return 0;
}

int file_system::write_sector(unsigned n, uint8_t *sector)
{
    fseek(file, SECTOR_SIZE * n, SEEK_SET);
    fwrite(sector, 1, SECTOR_SIZE, file);

    return 0;
}

int file_t::read(unsigned n, uint8_t *buffer)
{
    if(!entry)
        return -1;

    if(entry->size == 0)
        return 0;

    return n;
}

int file_t::write(unsigned n, uint8_t *buffer)
{
    if(!entry)
        return -1;



    return n;
}
