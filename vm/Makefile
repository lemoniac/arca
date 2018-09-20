CPPFLAGS=--std=c++11 -I../cpu/ -I../filesystem -g
OBJS=gpu.o vm.o bios.o ../filesystem/filesystem.o

emu: emu.cpp $(OBJS) ../cpu/opcodes.h
	g++ -o emu emu.cpp $(OBJS) $(CPPFLAGS) -lSDL2

clean:
	rm -f *.o
	rm -f emu