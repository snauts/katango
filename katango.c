typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

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

void irq_handler(void) {
}

void game_startup(void) {
}

