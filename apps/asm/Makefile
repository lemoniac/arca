CPPFLAGS=--std=c++11 -I../../cpu/ -g
OBJS=asm.o struct.o

asm: $(OBJS) ../../cpu/opcodes.h
	g++ -o asm $(OBJS) $(CPPFLAGS)

clean:
	rm -f asm $(OBJS)
