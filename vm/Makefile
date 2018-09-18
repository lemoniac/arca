CPPFLAGS=--std=c++11 -I../cpu/
OBJS=gpu.o vm.o bios.o

emu: emu.cpp $(OBJS) ../cpu/opcodes.h
	g++ -g -o emu emu.cpp $(OBJS) $(CPPFLAGS) -lSDL2

clean:
	rm emu