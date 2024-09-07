all: build

run: build
	fceux katango.nes

build:
	ca65 katango.s -o asm.o
	cc65 katango.c -o code.s
	ca65 code.s -o code.o
	ld65 -o katango.nes -C katango.cfg asm.o code.o

clean:
	rm -f *.o code.s katango.nes
