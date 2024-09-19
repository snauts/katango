#include <stddef.h>

#define FADE(f, l)	((l & 0xff) | (f << 8))
#define NOTE(n, o, l)	((l & 0xffff) | (n << 16) | (o << 24))

#define END		(0xff << 24)
#define DONE		0xdeadbeef
#define P(l)		(l & 0xff)
#define C(o, l)		NOTE(0x0, o, l)
#define Cs(o, l)	NOTE(0x1, o, l)
#define D(o, l)		NOTE(0x2, o, l)
#define Ds(o, l)	NOTE(0x3, o, l)
#define E(o, l)		NOTE(0x4, o, l)
#define F(o, l)		NOTE(0x5, o, l)
#define Fs(o, l)	NOTE(0x6, o, l)
#define G(o, l)		NOTE(0x7, o, l)
#define Gs(o, l)	NOTE(0x8, o, l)
#define A(o, l)		NOTE(0x9, o, l)
#define As(o, l)	NOTE(0xa, o, l)
#define B(o, l)		NOTE(0xb, o, l)

void print_level(char *name, char **level, int *height, void **sheet);
void print_sheet(const char *name, void **sheet);

void save_habanera(void);
void save_lunnaja(void);
