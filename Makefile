CPPFLAGS=--std=c++11

all: asm emu

asm:
	cd apps/asm; $(MAKE) asm && cp asm ../..

emu:
	cd vm; $(MAKE) emu && cp emu ..

clean:
	cd apps/asm; $(MAKE) clean
	cd vm; $(MAKE) clean
	rm asm
	rm emu