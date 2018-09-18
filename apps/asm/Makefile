CPPFLAGS=--std=c++11 -I../../cpu/

asm: asm.cpp ../../cpu/opcodes.h
	g++ -g -o asm asm.cpp $(CPPFLAGS)

clean:
	rm -f asm
