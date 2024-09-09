typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#include "title.hdr"

#define MEM_WR(a, x)	(* (volatile byte *) (a) = (x))

#define SIZE(array)	(sizeof(array) / sizeof(*(array)))

#define PPUADDR(x) \
    MEM_WR(0x2006, ((x) >> 8)); \
    MEM_WR(0x2006, ((x) & 0xff))

#define PPUDATA(x) \
    MEM_WR(0x2007, x)

extern byte oam_buffer[256];

static const byte palette[] = {
    0x0F, 0x03, 0x13, 0x23,
    0x0F, 0x0F, 0x0F, 0x0F,
    0x0F, 0x08, 0x18, 0x28,
    0x0F, 0x09, 0x19, 0x29,

    0x0F, 0x06, 0x16, 0x30,
    0x0F, 0x27, 0x28, 0x10,
    0x0F, 0x06, 0x16, 0x37,
    0x0F, 0x06, 0x16, 0x17,
};

void setup_palette(void) {
    byte i;
    PPUADDR(0x3f00);
    for (i = 0; i < SIZE(palette); i++) {
	PPUDATA(palette[i]);
    }
}

#pragma bss-name (push, "ZEROPAGE")

static volatile word ppu_addr;
static volatile byte ppu_count;
static volatile byte ppu_buffer[32];

#pragma bss-name (pop)

void irq_handler(void) {
    byte i;
    PPUADDR(ppu_addr);
    for (i = 0; i < ppu_count; i++) {
	PPUDATA(ppu_buffer[i]);
    }
    ppu_count = 0;
}

static void ppu_update_row(void) {
    ppu_count = 32;
    while (ppu_count > 0) { }
    ppu_addr += 32;
}

static void wipe_screen(void) {
    byte i;
    ppu_addr = 0x2020;
    for (i = 0; i < 32; i++) {
	ppu_buffer[i] = 0;
    }
    for (i = 0; i < 28; i++) {
	ppu_update_row();
    }
}

static void draw_image(const byte *data) {
    byte x, y;
    byte equal = 0;
    byte diffs = 0;
    ppu_addr = 0x2020;
    for (y = 0; y < 28; y++) {
	x = 0;
	while (x < 32) {
	    if (equal > 0) {
		ppu_buffer[x++] = *data;
		if (--equal > 0) continue;
	    }
	    else if (diffs > 0) {
		ppu_buffer[x++] = *data;
		diffs--;
	    }
	    else if (*data & 0x80) {
		equal = *data & ~0x80;
	    }
	    else {
		diffs = *data;
	    }
	    data++;
	}
	ppu_update_row();
    }
}

void game_startup(void) {
    wipe_screen();
    draw_image(title_data);
}
