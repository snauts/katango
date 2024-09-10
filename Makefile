CFLAGS = --nostdinc --nostdlib --no-std-crt0 --no-zp-spill --opt-code-speed
LFLAGS = -b CODE=0x8000 -b VECTOR=0xfffa

all: build

run: build
	fceux katango.nes

build:
	gcc pcx-dump.c -o pcx-dump
	./pcx-dump -r tiles.chr
	./pcx-dump -t title.pcx
	./pcx-dump -p tiles.chr
	./pcx-dump -s sprites.pcx
	@echo Compile katango.c
	@sdcc -mmos6502 $(CFLAGS) katango.c -c
	@echo Link katango.ihx
	@sdld $(LFLAGS) -m -i katango.ihx katango.rel
	hex2bin katango.ihx > /dev/null
	cat header.bin katango.bin tiles.chr sprites.chr > katango.nes

clean:
	rm -f *.o *.hdr *.chr code.s pcx-dump katango.nes
