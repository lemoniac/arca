#include <stdio.h>
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include "opcodes.h"
#include "vm.h"
#include "gpu.h"

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "emu file [diskimage]\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "rt");
    if(!file)
    {
        fprintf(stderr, "error loading file %s\n", argv[1]);
        return 1;
    }

    VM vm;

    if(argc == 3)
        vm.setDisk(argv[2]);

    //fread(&vm.codesize,4, 1, file);
    //fread(&vm.datasize,4, 1, file);

    fseek(file, 0, SEEK_END);
    vm.codesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(vm.RAM, vm.codesize, 1, file);
    //fread(vm.data, vm.datasize, 1, file);

    vm.init();
    vm.run();

    SDL_Delay(5000);

    return 0;
}
