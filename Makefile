all: build

run: build
	fceux katango.nes

build:
	gcc pcx-dump.c -o pcx-dump
	./pcx-dump -t tiles.pcx > tiles.chr
	./pcx-dump -t sprites.pcx > sprites.chr
	ca65 katango.s -o asm.o
	cc65 katango.c -o code.s
	ca65 code.s -o code.o
	ld65 -o katango.nes -C katango.cfg asm.o code.o

clean:
	rm -f *.o *.chr code.s pcx-dump katango.nes
