typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

void sdcc_deps(void) __naked {
    __asm__(".area ZP (PAG)");
    __asm__("REGTEMP:		.ds 8");
    __asm__("DPTR:		.ds 2");
    __asm__("_ppu_addr:		.ds 2");
    __asm__("_ppu_count:	.ds 1");
    __asm__("_ppu_buffer:	.ds 32");
    __asm__("_counter:		.ds 1");

    __asm__(".area OAM (PAG)");
    __asm__("_oam:		.ds 256");

    __asm__(".area CODE");
}

void irq(void) __naked {
    __asm__("rti");
}

void nmi(void) __naked {
    __asm__("pha");
    __asm__("txa");
    __asm__("pha");
    __asm__("tya");
    __asm__("pha");

    __asm__("jsr _irq_handler");

    __asm__("pla");
    __asm__("tay");
    __asm__("pla");
    __asm__("tax");
    __asm__("pla");

    __asm__("rti");
}

void rst(void) __naked {
    __asm__("sei");
    __asm__("cld");
    __asm__("ldx #0xff");
    __asm__("txs");

    __asm__("jsr _game_startup");
}

#include "title.hdr"

#define BIT(n)		(((byte) 1) << (n))

#define MEM_RD(a)	(* (volatile byte *) (a))
#define MEM_WR(a, x)	(* (volatile byte *) (a) = (x))

#define SIZE(array)	(sizeof(array) / sizeof(*(array)))

#define PPUCTRL(x)	MEM_WR(0x2000, x)
#define PPUMASK(x)	MEM_WR(0x2001, x)
#define PPUSTATUS()	MEM_RD(0x2002)
#define OAMADDR(x)	MEM_WR(0x2003, x)
#define OAMDATA(x)	MEM_WR(0x2004, x)
#define PPUSCROLL(x)	MEM_WR(0x2005, x)
#define PPUADDR(x)	MEM_WR(0x2006, x)
#define PPUDATA(x)	MEM_WR(0x2007, x)

#define NOISE_VL(x)	MEM_WR(0x400C, x)
#define NOISE_LO(x)	MEM_WR(0x400E, x)
#define NOISE_HI(x)	MEM_WR(0x400F, x)

#define DMCFREQ(x)	MEM_WR(0x4010, x)
#define OAMDMA(x)	MEM_WR(0x4014, x)
#define SND_CHN(x)	MEM_WR(0x4015, x)
#define JOY1(x)		MEM_WR(0x4016, x)
#define JOY2(x)		MEM_WR(0x4017, x)

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
}

static void clear_palette(void) {
    PPUADDR(0x3f);
    PPUADDR(0x00);
    for (byte i = 0; i < 32; i++) {
	PPUDATA(0x0f);
    }
}

extern byte oam[256];

static void init_oam_data(void) {
    byte i = 0;
    do {
	oam[i++] = 255;
    } while (i != 0);
}

static void ppu_ctrl(void) {
    PPUSCROLL(0x00);
    PPUSCROLL(0x00);
    PPUCTRL(BIT(7) | BIT(3));
}

static void hw_init(void) {
    JOY2(0x40);
    PPUCTRL(0x00);
    PPUMASK(0x00);
    DMCFREQ(0x00);
    SND_CHN(0x0f);

    wait_vblank();
    init_oam_data();
    wait_vblank();
    clear_palette();
    wait_vblank();
    ppu_ctrl();
}

extern volatile word ppu_addr;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile byte counter;

void irq_handler(void) {
    byte i;
    PPUADDR(ppu_addr >> 8);
    PPUADDR(ppu_addr & 0xff);
    for (i = 0; i < ppu_count; i++) {
	PPUDATA(ppu_buffer[i]);
    }
    ppu_count = 0;
    counter++;

    PPUMASK(0x00);
    OAMADDR(0x00);
    OAMDMA(0x02); /* oam addr high byte */
    ppu_ctrl();
    PPUMASK(0x1e);
}

static void delay(byte ticks) {
    byte next = counter + ticks;
    while (next != counter) { }
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
    ppu_addr = 0x2000;
    for (i = 0; i < 32; i++) {
	ppu_buffer[i] = 0;
    }
    for (i = 0; i < 30; i++) {
	ppu_update_row();
    }
}

static void decode_rle(const byte *data, word where, byte rows) {
    byte x, y;
    byte equal = 0;
    byte diffs = 0;
    ppu_addr = where;
    for (y = 0; y < rows; y++) {
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

static void draw_screen(const byte *data) {
    decode_rle(data, 0x2000, 30);
}

static void attr_screen(const byte *data) {
    decode_rle(data, 0x23c0, 2);
}

static void shiver_palette(byte idx, byte color) {
    NOISE_VL(idx);
    for (byte i = 0; i < 4; i++) {
	update_palette(idx, 0x0f);
	delay(2);
	update_palette(idx, color);
	delay(2);
	NOISE_LO(i << 2);
	NOISE_HI(0x08);
    }
}

static void rotate_palette_border(byte wait, char snd) {
    static const byte roll[] = {
	0x0f, 0x04, 0x14,
	0x0f, 0x0f, 0x04,
	0x0f, 0x0f, 0x0f,

	0x24, 0x0f, 0x0f,
	0x14, 0x24, 0x0f,
	0x04, 0x14, 0x24,
    };

    NOISE_VL(0x30 | (6 - snd));
    NOISE_HI(0x08);

    for (byte i = 0; i < sizeof(roll); i += 3) {
	for (byte n = 0; n < 3; n++) {
	    update_palette(5 + n, roll[i + n]);
	}
	NOISE_LO(snd + 11);
	snd = snd - 1;
    }
    delay(wait);
}

static void animate_title_text(void) {
    shiver_palette(0x07, 0x04);
    shiver_palette(0x06, 0x04);
    update_palette(0x07, 0x14);
    shiver_palette(0x05, 0x04);
    rotate_palette_border(2, 0);
    update_palette(0x03, 0x03);
    rotate_palette_border(1, 1);
    update_palette(0x02, 0x03);
    update_palette(0x03, 0x13);
    rotate_palette_border(0, 2);
    update_palette(0x01, 0x03);
    update_palette(0x02, 0x13);
    update_palette(0x03, 0x23);
    rotate_palette_border(2, 3);
    update_palette(0x0e, 0x16);
    rotate_palette_border(1, 4);
    update_palette(0x0d, 0x16);
    update_palette(0x0e, 0x26);
    NOISE_VL(0);
}

void game_startup(void) {
    hw_init();

    wipe_screen();
    draw_screen(title_data);
    attr_screen(title_attr);
    animate_title_text();

    for (;;) { }
}

/* must be very last */
void jump_vectors(void) __naked {
    __asm__(".area VECTOR (PAG)");
    __asm__("nmi_ptr::	.dw _nmi");
    __asm__("rst_ptr::	.dw _rst");
    __asm__("irq_ptr::	.dw _irq");
}
