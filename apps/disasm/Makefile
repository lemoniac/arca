CFLAGS=-I../../cpu/

disasm: disasm.c ../../cpu/opcodes.h
	gcc -g -o disasm disasm.c $(CFLAGS)

clean:
	rm -f disasm
