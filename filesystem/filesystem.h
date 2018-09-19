#ifndef ARCA__FILESYSTEM__
#define ARCA__FILESYSTEM__

#include <stdint.h>
#include <stdio.h>

#define MAX_FILENAME_SIZE 8
#define SECTOR_SIZE 512

struct file_entry {
    char filename[MAX_FILENAME_SIZE];
    uint16_t size;
    uint16_t first_block;
    char padding[4];
};

#define ENTRIES_PER_SECTOR SECTOR_SIZE / sizeof(file_entry)

#define ENTRIES_PER_BLOCK 7

struct block_t {
    uint16_t sector[ENTRIES_PER_BLOCK];
    uint16_t next_block;
};

struct file_t {
    file_entry *entry;

    int seek(int n);
    int read(unsigned n, uint8_t *buffer);
    int write(unsigned n, uint8_t *buffer);
};

struct file_system {
    int init(const char *filesystem);
    void finalize();

    int create_file(const char *filename);
    int remove_file(const char *filename);

    file_entry *entry(unsigned n);

    int empty_entry();
    int empty_block();

    int find_file(const char *name);

    int read_sector(unsigned n, uint8_t *sector);
    int write_sector(unsigned n, uint8_t *sector);

    FILE *file;
    file_entry entries[ENTRIES_PER_SECTOR];
    block_t blocks[ENTRIES_PER_SECTOR];
    uint8_t sectors[65536 / SECTOR_SIZE];
};


#endif//ARCA__FILESYSTEM__
