void sdcc_deps(void) __naked {
    __asm__(".area ZP (PAG)");
    __asm__("REGTEMP::	.ds 8");
    __asm__("DPTR::	.ds 2");
    __asm__(".area CODE");
}

void irq(void) __naked {
    __asm__("rti");
}

void nmi(void) __naked {
    __asm__("rti");
}

void rst(void) __naked {
    __asm__("sei");
    __asm__("cld");
    __asm__("ldx #0xff");
    __asm__("txs");
}

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

void setup_palette(void) {
    byte i;
    PPUADDR(0x3f00);
    for (i = 0; i < 32; i++) {
	PPUDATA(0xF);
    }
}

static volatile word ppu_addr;
static volatile byte ppu_count;
static volatile byte ppu_buffer[32];

static volatile byte counter;

void irq_handler(void) {
    byte i;
    PPUADDR(ppu_addr);
    for (i = 0; i < ppu_count; i++) {
	PPUDATA(ppu_buffer[i]);
    }
    ppu_count = 0;
    counter++;
}

static void delay(byte ticks) {
    byte now = counter;
    while (counter - now < ticks) { }
}

static void update_palette(byte idx, byte val) {
    ppu_addr = 0x3f00 | idx;
    ppu_buffer[ppu_count] = val;
    ppu_count++;
    while (ppu_count > 0) { }
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

    update_palette(1, 0x03);
    update_palette(2, 0x13);
    update_palette(2, 0x23);
}

/* must be very last */
void jump_vectors(void) __naked {
    __asm__(".area VECTOR (PAG)");
    __asm__("nmi_ptr::	.dw _nmi");
    __asm__("rst_ptr::	.dw _rst");
    __asm__("irq_ptr::	.dw _irq");
}
