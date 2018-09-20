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
    flush();
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

    flush();

    return n;
}

int file_system::remove_file(const char *filename)
{
    int n = find_file(filename);
    if(n < 0)
        return -1;

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

int file_system::find_file(const char *name) const
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

int file_system::write_sector(unsigned n, const uint8_t *sector)
{
    fseek(file, SECTOR_SIZE * n, SEEK_SET);
    fwrite(sector, 1, SECTOR_SIZE, file);

    return 0;
}

file_t file_system::open(const char *filename)
{
    file_t f = {};

    int entry = find_file(filename);
    if(entry < 0)
        entry = create_file(filename);

    f.entry_id = entry;
    f.fs = this;

    return f;
}

void file_system::flush()
{
    write_sector(0, (uint8_t *)entries);
    write_sector(1, (uint8_t *)blocks);
}

int file_system::allocate_sector()
{
    for(unsigned i = 0; i < 65536 / SECTOR_SIZE; i++)
    {
        if(sectors[i] == 0)
        {
            sectors[i] = 1;
            return i;
        }
    }

    return -1;
}


unsigned file_t::size() const
{
    return fs->entries[entry_id].size;
}

bool file_t::empty() const
{
    return fs->entries[entry_id].size == 0;
}

int file_t::seek(int n)
{
    if(n > size())
        n = size();
}

int file_t::read(unsigned n, uint8_t *buffer)
{
    if(!fs || entry_id < 0)
        return -1;

    if(empty())
        return 0;

    return n;
}

int file_t::write(unsigned n, const uint8_t *buffer)
{
    if(!fs || entry_id < 0)
        return -1;

    unsigned sector_id = offset / SECTOR_SIZE;
    unsigned sector_off = offset % SECTOR_SIZE;

    file_entry &e = fs->entries[entry_id];
    block_t &b = fs->blocks[e.first_block];
    int s = b.sector[sector_id];
    if(s == 0)
    {
        s = fs->allocate_sector();
        if(s < 0)
            return -1;
        b.sector[sector_id] = s;
        fs->flush();
    }

    uint8_t tmp[SECTOR_SIZE];
    fs->read_sector(s, tmp);
    memcpy(tmp + sector_off, buffer, n);
    fs->write_sector(s, tmp);

    offset += n;

    if(offset > size())
        fs->entries[entry_id].size += n;

    return n;
}
