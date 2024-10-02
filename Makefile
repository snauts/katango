CFLAGS = --nostdinc --nostdlib --no-std-crt0 --no-zp-spill --opt-code-speed
LFLAGS = -b OAM=0x200 -b BSS=0x300 -b CODE=0x8000 -b VECTOR=0xfffa

TOOL_FILES = pcx-dump.c habanera.c lunnaja.c infernal.c

all: build

run: build
	fceux katango.nes

pal:
	TFLAGS=-DPAL make build

build:
	gcc $(TOOL_FILES) $(TFLAGS) -lm -o pcx-dump
	./pcx-dump -r tiles.chr
	./pcx-dump -t fonts.pcx
	./pcx-dump -t title.pcx
	./pcx-dump -t alley.pcx
	./pcx-dump -t ocean.pcx
	./pcx-dump -t flame.pcx
	./pcx-dump -t stars.pcx
	./pcx-dump -p tiles.chr
	./pcx-dump -s sprites.pcx
	./pcx-dump -m ? > music.hdr
	@echo Compile katango.c
	@sdcc -mmos6502 $(CFLAGS) katango.c -c
	@echo Link katango.ihx
	@sdld $(LFLAGS) -m -i katango.ihx katango.rel
	hex2bin -e prg katango.ihx > /dev/null
	cat tiles.chr sprites.chr > katango.chr
	cat header.rom katango.prg katango.chr > katango.nes

clean:
	rm -f *.asm *.ihx *.lst *.map *.rel *.sym *.chr *.hdr *.prg *.nes
	rm -f pcx-dump
