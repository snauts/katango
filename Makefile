CFLAGS = --nostdinc --nostdlib --no-std-crt0 --no-zp-spill --opt-code-speed
LFLAGS = -b OAM=0x200 -b BSS=0x300 -b CODE=0x8000 -b VECTOR=0xfffa

all: build

run: build
	fceux katango.nes

build:
	gcc pcx-dump.c -o pcx-dump.bin
	./pcx-dump.bin -r tiles.chr
	./pcx-dump.bin -t title.pcx
	./pcx-dump.bin -t alley.pcx
	./pcx-dump.bin -p tiles.chr
	./pcx-dump.bin -s sprites.pcx
	@echo Compile katango.c
	@sdcc -mmos6502 $(CFLAGS) katango.c -c
	@echo Link katango.ihx
	@sdld $(LFLAGS) -m -i katango.ihx katango.rel
	hex2bin katango.ihx > /dev/null
	cat header.rom katango.bin tiles.chr sprites.chr > katango.nes

clean:
	rm -f *.asm *.ihx *.lst *.map *.rel *.sym *.chr *.hdr *.bin *.nes
