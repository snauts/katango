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
    __asm__("_score:		.ds 5");
    __asm__("_lives:		.ds 1");
    __asm__("_height:		.ds 1");
    __asm__("_signal:		.ds 1");
    __asm__("_counter:		.ds 1");
    __asm__("_buttons:		.ds 1");
    __asm__("_distance:		.ds 1");
    __asm__("_position:		.ds 1");
    __asm__("_direction:	.ds 1");
    __asm__("_height_map:	.ds 7");

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

#define TRI_CR(x)	MEM_WR(0x4008, x)
#define TRI_LO(x)	MEM_WR(0x400A, x)
#define TRI_HI(x)	MEM_WR(0x400B, x)

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

#define FISH_SPRITES	128
#define WIND_SPRITES	244
#define CAT_SPRITES	220

extern byte oam[256];

extern volatile word ppu_addr;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile byte counter;
extern volatile byte signal;

extern byte score[5];

extern byte lives;
extern byte height;
extern byte buttons;
extern byte distance;
extern byte position;
extern byte direction;
extern byte height_map[7];

static byte wind_frame;
static byte wind_x_dir;
static byte wind_y_dir;

static byte fish_free;
static byte fish_dir;
static byte fish_left;
static byte fish_done;
static byte fish_snd;

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
    signal = 0;
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

static void memset(void *buf, byte val, word count) {
    byte *ptr = buf;
    while (count-- > 0) {
	*ptr++ = val;
    }
}

static void clear_palette(void) {
    PPUADDR(0x3f);
    PPUADDR(0x00);
    for (byte i = 0; i < 32; i++) {
	PPUDATA(0x0f);
    }
}

static void init_memory(void) {
    buttons = 0;
    counter = 0;
    ppu_count = 0;

    memset(oam, 255, 0x100);
}

static void update_cat(void);
static void reset_level(void) {
    position = 3;
    direction = 0;

    fish_free = 0;
    fish_dir = 0;
    fish_snd = 16;

    update_cat();
}

static void reset_game_state(void) {
    memset(score, 0, 5);
    lives = 9;

    memset(height_map, 208, 7);

    reset_level();
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
    signal = 1;
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
    memset(ppu_buffer, 0xf, 32);
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
    memset(oam, 255, 0x100);
    memset(ppu_buffer, 0, 32);
    for (byte i = 0; i < 32; i++) {
	ppu_update(32);
    }
    wipe_palette();
}

static const char special[] = " @:"; /* @ is heart symbol */
static byte digit_to_tile(byte c) {
    return sizeof(special) + c - 1;
}

static byte char_to_tile(char c) {
    byte sym;
    for (sym = 0; sym < sizeof(special) - 1; sym++) {
	if (special[sym] == c) return sym;
    }

    sym = sym + c;
    if (c >= '0' && c <= '9') {
	return sym - '0';
    }

    sym = sym + 10;
    if (c >= 'A' && c <= 'Z') {
	return sym - 'A';
    }
    if (c >= 'a' && c <= 'z') {
	return sym - 'a';
    }
    return 0;
}

static void print_msg(const char *msg, byte x, word y) {
    byte i = 0;
    ppu_addr = 0x2020 + (y << 5) + x;
    while (msg[i] != 0) {
	ppu_buffer[i] = char_to_tile(msg[i]);
	i++;
    }
    ppu_update(i);
}

static void check_vblank(void) {
    if (signal == 1) {
	print_msg("VBLANK", 0, 0);
	for (;;) { }
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
    0x0f, 0x15, 0x25, 0x35,
    0x0f, 0x07, 0x17, 0x09,
    0x0f, 0x00, 0x10, 0x20,
    0x0f, 0x06, 0x16, 0x26,

    0x0f, 0x0f, 0x0c, 0x38,
    0x0f, 0x12, 0x1c, 0x21,
    0x0f, 0x12, 0x13, 0x24,
};

static void setup_alley_palette(void) {
    setup_palette(alley_palette, 0, sizeof(alley_palette));
}

static const byte cat_pos[] = {
    24, 56, 88, 120, 152, 184, 216
};

static const byte cat_x[] = {
    0, 8, 0, 8, 0, 8, 0
};

static const byte cat_y[] = {
    16, 16, 8, 8, 0, 0
};

static const byte cat_s[] = {
    0, 1, 16, 17, 32, 33,
};

static const byte cat_img[] = {
    6, 4, 2, 0, 2, 4, 6,
};

static void add_wind(byte side) {
    byte i = WIND_SPRITES;
    byte offset = distance + side;
    for (byte n = 0; n < 6; n += 2) {
	oam[i++] = height - cat_y[n];
	oam[i++] = cat_s[n] + 8;
	oam[i++] = direction;
	oam[i++] = offset;
    }
    wind_frame = 0;

    NOISE_VL(0x14);
    NOISE_HI(0x00);
    NOISE_LO(0x00);
}

static void move_wind(void) {
    if (oam[WIND_SPRITES] != 255) {
	for (byte n = WIND_SPRITES; n != 0; n += 4) {
	    oam[n + 0] += wind_y_dir;
	    oam[n + 3] += wind_x_dir;
	}
	if (++wind_frame == 8) {
	    for (byte n = WIND_SPRITES; n != 0; n += 4) {
		oam[n] = 255;
	    }
	}
    }
}

static void update_cat(void) {
    byte prev = height;
    height = height_map[position];
    distance = cat_pos[position];

    if (height >= prev) {
	wind_y_dir = (height - prev) >> 3;
    }
    else {
	wind_y_dir = 0 - ((prev - height) >> 3);
    }
}

static void move_cat(void) {
    byte button = check_button();
    if (position < 6 && (button & CAT_RIGHT)) {
	direction = 0;
	wind_x_dir = 3;
	add_wind(8);
	position++;
	update_cat();
    }
    if (position > 0 && (button & CAT_LEFT)) {
	direction = BIT(6);
	wind_x_dir = 253;
	add_wind(0);
	position--;
	update_cat();
    }
}

static void place_cat(void) {
    byte i = CAT_SPRITES;
    byte s = cat_img[position];
    byte offset = direction ? 1 : 0;
    for (byte n = 0; n < 6; n++) {
	oam[i++] = height - cat_y[n];
	oam[i++] = cat_s[n] + s;
	oam[i++] = direction;
	oam[i++] = distance + cat_x[n + offset];
    }
}

static void emit_fish(byte pos) {
    if (fish_free < FISH_SPRITES) {
	oam[fish_free + 0] = 0;
	oam[fish_free + 1] = 48;
	oam[fish_free + 2] = fish_dir | 1;
	oam[fish_free + 3] = cat_pos[pos] + 4;
	fish_dir = fish_dir ^ BIT(6);
	fish_free = FISH_SPRITES;
    }
}

static void animate_fish(byte index) {
    byte sprite = oam[index];
    sprite = sprite < 55 ? sprite + 1 : 48;
    oam[index] = sprite;
}

static void sound_sfx(void) {
    if (fish_snd < 16) {
	static const byte sfx[] = {
	    0x10, 0x10, 0x10, 0x11,
	    0x20, 0x10, 0x20, 0x11,
	    0x40, 0x10, 0x40, 0x11,
	    0x80, 0x10, 0x80, 0x11,
	};
	TRI_CR(0x0f);
	TRI_LO(sfx[fish_snd++]);
	TRI_HI(sfx[fish_snd++]);
    }
}

static void lose_live(void) {
    if (lives > 0) {
	ppu_buffer[17 + lives] = 0;
    }
    fish_snd = 0;
    lives--;
}

static void animate_drop(byte index) {
    byte sprite = oam[index + 1];
    if (sprite < 64) {
	sprite = 64;
	lose_live();
    }
    else if (sprite < 67) {
	sprite++;
    }
    else {
	oam[index] = 255;
    }
    oam[index + 1] = sprite;
}

static void catch_fish(byte i, byte s) {
    if (fish_done != 255) {
	oam[fish_done] = 255;
    }
    fish_left = 25;
    fish_done = i;
    oam[i + 1] = s;
    oam[i + 2] = 2;
}

static void fish_expire(byte i) {
    if (--fish_left == 0) {
	fish_done = 255;
	oam[i] = 255;
    }
}

static void inc_score(byte amount) {
    for (byte i = 4; i != 255; i--) {
	score[i] += amount;
	if (score[i] >= 10) {
	    score[i] -= 10;
	    amount = 1;
	}
	else break;
    }
}

static const byte fish_score[] = {
    0, 1, 1, 1, 2, 2, 3, 3, 5, 3, 3, 2, 2, 1, 1, 1
};

static const byte fish_bonus[] = {
    0, 71, 71, 71, 70, 70, 69, 69, 68, 69, 69, 70, 70, 71, 71, 71
};

static void move_fish(void) {
    byte animate = (counter & 3) == 0;
    for (byte i = 0; i < FISH_SPRITES; i += 4) {
	if (oam[i] == 255) {
	    fish_free = i;
	}
	else if (oam[i + 1] >= 68) {
	    fish_expire(i);
	}
	else {
	    byte index = oam[i + 3] >> 5;
	    byte range = height_map[index] - oam[i];

	    if (range == 0) {
		if (animate) animate_drop(i);
	    }
	    else if (index == position && range < 16) {
		catch_fish(i, fish_bonus[range]);
		inc_score(fish_score[range]);
	    }
	    else {
		if (animate) animate_fish(i + 1);
		oam[i]++;
	    }
	}
    }
}

static void score_to_buffer(byte offset) {
    for (byte i = 0; i < 5; i++) {
	ppu_buffer[offset + i] = digit_to_tile(score[i]);
    }
}

static void update_score(void) {
    score_to_buffer(6);
    ppu_addr = 0x2021;
    ppu_count = 27;
}

static void game_over(void) {
    wipe_screen();

    static const byte game_over_palette[] = { 0x0f, 0x16, 0x26, 0x36 };
    setup_palette(game_over_palette, 0, sizeof(game_over_palette));

    print_msg("GAME OVER", 12, 12);

    score_to_buffer(0);
    ppu_addr = 0x21ee;
    ppu_update(5);

    wait_start_button();
}

static void start_game_loop(void) {
    while (lives <= 9) {
	wait_vblank();
	move_wind();
	move_cat();
	place_cat();
	move_fish();
	sound_sfx();
	update_score();
	check_vblank();
    }
    game_over();
}

static void print_score_n_lives(void) {
    print_msg("SCORE:00000 LIVES:@@@@@@@@@", 1, 0);
}

void game_startup(void) {
    hw_init();

    for (;;) {
	wipe_screen();
	attr_screen(title_attr);
	draw_screen(title_data);
	animate_title_text();
	wait_start_button();

	reset_game_state();

	wipe_screen();
	setup_alley_palette();
	attr_screen(alley_attr);
	draw_screen(alley_data);
	print_score_n_lives();
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
