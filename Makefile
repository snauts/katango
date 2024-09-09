CFLAGS = -O --static-locals

all: build

run: build
	fceux katango.nes

build:
	gcc pcx-dump.c -o pcx-dump
	./pcx-dump -r tiles.chr
	./pcx-dump -t title.pcx
	./pcx-dump -p tiles.chr
	./pcx-dump -s sprites.pcx
	ca65 katango.s -o asm.o
	cc65 katango.c $(CFLAGS) -o code.s
	ca65 code.s -o code.o
	ld65 -o katango.nes -C katango.cfg asm.o code.o

clean:
	rm -f *.o *.chr code.s pcx-dump katango.nes
