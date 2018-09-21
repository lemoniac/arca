CPPFLAGS=--std=c++11

all: filesystem.o asm emu

asm:
	cd apps/asm; $(MAKE) asm && cp asm ../..

emu:
	cd vm; $(MAKE) emu && cp emu ..

filesystem.o:
	cd filesystem; $(MAKE)

clean:
	cd apps/asm; $(MAKE) clean
	cd vm; $(MAKE) clean
	cd filesystem; $(MAKE) clean
	rm -f asm
	rm -f emu