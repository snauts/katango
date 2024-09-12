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
    __asm__("_buttons:		.ds 1");
    __asm__("_position:		.ds 1");
    __asm__("_direction:	.ds 1");

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
#include "alley.hdr"

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
#define JOY1_WR(x)	MEM_WR(0x4016, x)
#define JOY1_RD()	MEM_RD(0x4016)
#define JOY2(x)		MEM_WR(0x4017, x)

#define BUTTON_A	BIT(7)
#define BUTTON_B	BIT(6)
#define BUTTON_SELECT	BIT(5)
#define BUTTON_START	BIT(4)
#define BUTTON_UP	BIT(3)
#define BUTTON_DOWN	BIT(2)
#define BUTTON_LEFT	BIT(1)
#define BUTTON_RIGHT	BIT(0)

#define CAT_RIGHT	(BUTTON_RIGHT | BUTTON_A)
#define CAT_LEFT	(BUTTON_LEFT  | BUTTON_B)

extern byte oam[256];

extern volatile word ppu_addr;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile byte counter;

extern byte buttons;
extern byte position;
extern byte direction;

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
}

static byte check_button(void) {
    JOY1_WR(0x00);
    JOY1_WR(0x01);

    byte press, state = 0;
    for (byte i = 0; i < 8; i++) {
	state = state << 1;
	state |= JOY1_RD() & 1;
    }
    press = (buttons ^ state) & state;
    buttons = state;
    return press;
}

static void wait_start_button(void) {
    while (!(check_button() & BUTTON_START)) { }
}

static void clear_palette(void) {
    PPUADDR(0x3f);
    PPUADDR(0x00);
    for (byte i = 0; i < 32; i++) {
	PPUDATA(0x0f);
    }
}

static void init_memory(void) {
    byte i = 0;
    buttons = 0;
    counter = 0;
    position = 3;
    direction = 0;
    ppu_count = 0;
    do { oam[i++] = 255; } while (i != 0);
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
    init_memory();
    wait_vblank();
    clear_palette();
    wait_vblank();
    ppu_ctrl();
}

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

static void ppu_update(byte amount) {
    ppu_count = amount;
    while (ppu_count > 0) { }
    ppu_addr += amount;
}

static void wipe_palette(void) {
    ppu_addr = 0x3f00;
    for (byte i = 0; i < 32; i++) {
	ppu_buffer[i] = 0xf;
    }
    ppu_update(32);
}

static void setup_palette(const byte *ptr, byte offset, byte amount) {
    ppu_addr = 0x3f00 + offset;
    for (byte i = 0; i < amount; i++) {
	ppu_buffer[i] = ptr[i];
    }
    ppu_update(amount);
}

static void wipe_screen(void) {
    ppu_addr = 0x2000;
    for (byte i = 0; i < 32; i++) {
	ppu_buffer[i] = 0;
    }
    for (byte i = 0; i < 30; i++) {
	ppu_update(32);
    }
    wipe_palette();
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
	ppu_update(32);
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

static void rotate_palette_border(byte wait, byte snd) {
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

    byte period = snd + 11;
    for (byte i = 0; i < sizeof(roll); i += 3) {
	for (byte n = 0; n < 3; n++) {
	    update_palette(5 + n, roll[i + n]);
	}
	NOISE_LO(period--);
	delay(snd);
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
    rotate_palette_border(3, 3);
    rotate_palette_border(3, 4);
    update_palette(0x0e, 0x26);
    update_palette(0x0d, 0x16);
    NOISE_VL(0);
}

static const byte alley_palette[] = {
    0x0f, 0x00, 0x10, 0x20,
    0x0f, 0x07, 0x17, 0x09,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x06, 0x16, 0x26,
};

static const byte cat_pos[] = {
    24, 56, 88, 120, 152, 184, 216
};

static const byte cat_x[] = {
    0, 8, 0, 8, 0, 8, 0
};

static const byte cat_y[] = {
    192, 192, 200, 200, 208, 208
};

static const byte cat_s[] = {
    0, 1, 16, 17, 32, 33,
};

static const byte cat_img[] = {
    6, 4, 2, 0, 2, 4, 6,
};

static byte wind_state;
static void add_wind(void) {
    byte i = 24;
    byte attribute;
    byte offset = cat_pos[position];
    offset += direction ? 24 : 240;
    for (byte n = 0; n < 6; n += 2) {
	oam[i++] = cat_y[n];
	oam[i++] = cat_s[n] + 8;
	oam[i++] = direction;
	oam[i++] = offset;
    }
    wind_state = 8;

    NOISE_VL(0x14);
    NOISE_HI(0x00);
    NOISE_LO(0x00);
}

static void move_wind(void) {
    if (oam[24] != 255) {
	byte move = direction ? 254 : 2;
	for (byte n = 27; n < 36; n += 4) {
	    oam[n] += move;
	}
	if (--wind_state == 0) {
	    for (byte n = 24; n < 36; n += 4) {
		oam[n] = 255;
	    }
	}
    }
}

static void move_cat(void) {
    byte button = check_button();
    if (position < 6 && (button & CAT_RIGHT)) {
	direction = 0;
	position++;
	add_wind();
    }
    if (position > 0 && (button & CAT_LEFT)) {
	direction = BIT(6);
	position--;
	add_wind();
    }
}

static void place_cat(void) {
    byte i = 0;
    byte x = cat_pos[position];
    byte sprite_idx = cat_img[position];
    byte offset = direction ? 1 : 0;
    for (byte n = 0; n < 6; n++) {
	oam[i++] = cat_y[n];
	oam[i++] = cat_s[n] + sprite_idx;
	oam[i++] = direction;
	oam[i++] = x + cat_x[n + offset];
    }
}

static void start_game_loop(void) {
    for (;;) {
	move_cat();
	place_cat();
	move_wind();
	wait_vblank();
    }
}

void game_startup(void) {
    hw_init();

    for (;;) {
	wipe_screen();
	attr_screen(title_attr);
	draw_screen(title_data);
	animate_title_text();
	wait_start_button();

	wipe_screen();
	setup_palette(alley_palette, 0, 16);
	attr_screen(alley_attr);
	draw_screen(alley_data);
	update_palette(18, 0x0c);
	update_palette(19, 0x38);
	start_game_loop();
    }
}

/* must be very last */
void jump_vectors(void) __naked {
    __asm__(".area VECTOR (PAG)");
    __asm__("nmi_ptr::	.dw _nmi");
    __asm__("rst_ptr::	.dw _rst");
    __asm__("irq_ptr::	.dw _irq");
}
