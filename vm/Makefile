CPPFLAGS=--std=c++11 -I../cpu/

emu: emu.cpp ../cpu/opcodes.h
	g++ -g -o emu emu.cpp $(CPPFLAGS) -lSDL2

clean:
	rm emu