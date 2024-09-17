#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#define MAX_SIZE 4096

static char *file_name;

static unsigned char tileset[MAX_SIZE];
static int tileset_size;

struct Header {
    unsigned short w, h;
} header;

static int file_size(const char *file) {
    struct stat st;
    return stat(file, &st) != 0 ? -1 : st.st_size;
}

static void replace_ext(char *name, const char *ext) {
    strcpy(name, file_name);
    strcpy(name + strlen(file_name) - strlen(ext), ext);
}

static unsigned char *read_pcx(const char *file) {
    int palette_offset = 16;
    int size = file_size(file);

    if (size < 0) {
	fprintf(stderr, "ERROR while opening PCX-file \"%s\"\n", file);
	return NULL;
    }

    unsigned char *buf = malloc(size);
    int in = open(file, O_RDONLY);
    read(in, buf, size);
    close(in);

    header.w = (* (unsigned short *) (buf + 0x8)) + 1;
    header.h = (* (unsigned short *) (buf + 0xa)) + 1;
    if (buf[3] == 8) palette_offset = size - 768;
    int unpacked_size = header.w * header.h / (buf[3] == 8 ? 1 : 2);
    unsigned char *pixels = malloc(unpacked_size);

    int i = 128, j = 0;
    while (j < unpacked_size) {
	if ((buf[i] & 0xc0) == 0xc0) {
	    int count = buf[i++] & 0x3f;
	    while (count-- > 0) {
		pixels[j++] = buf[i];
	    }
	    i++;
	}
	else {
	    pixels[j++] = buf[i++];
	}
    }

    free(buf);
    return pixels;
}

static unsigned char get_bit_line(unsigned char *buf, int plane) {
    unsigned char byte = 0;
    for (int i = 0; i < 8; i++) {
	if (buf[i] & plane) {
	    byte |= (0x80 >> i);
	}
    }
    return byte;
}

static void get_bit_plane(unsigned char *buf, unsigned char *ptr, int plane) {
    for (int y = 0; y < 8; y++) {
	ptr[y] = get_bit_line(buf + y * header.w, plane);
    }
}

static void save_sprites(unsigned char *buf) {
    char tile_name[256];
    replace_ext(tile_name, "chr");
    int fd = open(tile_name, O_CREAT | O_RDWR, 0644);
    for (int y = 0; y < header.h; y += 8) {
	for (int x = 0; x < header.w; x += 8) {
	    unsigned char result[16];
	    unsigned char *ptr = buf + y * header.w + x;
	    get_bit_plane(ptr, result + 0, 1);
	    get_bit_plane(ptr, result + 8, 2);
	    write(fd, result, 16);
	}
    }
    close(fd);
}

static int reset_tiles(void) {
    unsigned char buf[16];
    int fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, 0644);
    memset(buf, 0, sizeof(buf));
    write(fd, buf, sizeof(buf));
    close(fd);
    return 0;
}

static int pad_tiles(void) {
    int size = file_size(file_name);

    if (size < 0 || size > MAX_SIZE) {
	fprintf(stderr, "ERROR invalid tile file \"%s\"\n", file_name);
	return -ENOENT;
    }

    unsigned char *buf = malloc(MAX_SIZE);
    memset(buf, 0, MAX_SIZE);

    int fd = open(file_name, O_RDWR, 0644);
    read(fd, buf, size);
    lseek(fd, 0, SEEK_SET);
    write(fd, buf, MAX_SIZE);
    close(fd);
    return 0;
}

static void load_tileset(void) {
    tileset_size = file_size("tiles.chr");
    int fd = open("tiles.chr", O_RDWR, 0644);
    read(fd, tileset, tileset_size);
    close(fd);
}

static void save_tileset(void) {
    int fd = open("tiles.chr", O_RDWR, 0644);
    write(fd, tileset, tileset_size);
    close(fd);
}

static int tile_cmp(unsigned char *a, unsigned char *b) {
    for (int i = 0; i < 16; i++) {
	if (a[i] != b[i]) return 0;
    }
    return 1;
}

static int look_up_tile(unsigned char *buf) {
    for (int i = 0; i < tileset_size; i += 16) {
	if (tile_cmp(buf, tileset + i)) {
	    return i / 16;
	}
    }
    if (tileset_size < MAX_SIZE) {
	memcpy(tileset + tileset_size, buf, 16);
	tileset_size += 16;
    }
    return (tileset_size / 16) - 1;
}

static int get_equal(unsigned char *buf, int max) {
    for (int i = 1; i < max; i++) {
	if (buf[0] != buf[i]) {
	    return i;
	}
    }
    return max;
}

static int get_diffs(unsigned char *buf, int max) {
    for (int i = 0; i < max; i++) {
	if (get_equal(buf + i, MIN(max, 3)) > 2) {
	    return i;
	}
    }
    return max;
}

static void print_hex(FILE *fp, unsigned char x, int *size) {
    fprintf(fp, " 0x%02x,", x);
    if (((*size)++ & 0x7) == 0x7) {
	fprintf(fp, "\n");
    }
}

static int generate_data_map(unsigned char *buf, unsigned char *map) {
    int count = 0;

    for (int y = 0; y < header.h; y += 8) {
	for (int x = 0; x < header.w; x += 8) {
	    unsigned char result[16];
	    unsigned char *ptr = buf + y * header.w + x;
	    get_bit_plane(ptr, result + 0, 1);
	    get_bit_plane(ptr, result + 8, 2);
	    map[count++] = look_up_tile(result);
	}
    }

    return count;
}

static int look_up_attr(unsigned char *buf) {
    int attr = 0;
    for (int y = 0; y < 16; y++) {
	for (int x = 0; x < 16; x++) {
	    int offset = x + y * header.w;
	    attr = MAX(attr, (buf[offset] >> 2) & 3);
	}
    }
    return attr;
}

static int attr_block(unsigned char *buf) {
    int result = 0, offset[] = {
	header.w * 16 + 16, header.w * 16, 16, 0
    };
    for (int i = 0; i < 4; i++) {
	result = (result << 2) | look_up_attr(buf + offset[i]);
    }
    return result;
}

static int generate_attr_map(unsigned char *buf, unsigned char *map) {
    int i = 0;
    memset(map, 0, 64);

    for (int y = 0; y < header.h; y += 32) {
	for (int x = 0; x < header.w; x += 32) {
	    map[i++] = attr_block(buf + y * header.w + x);
	}
    }

    return 64;
}

static void compress_map(FILE *fp, unsigned char *map, int count) {
    int size = 0;
    int offset = 0;
    while (offset < count) {
	int max = MIN(count - offset, 127);
	unsigned char *ptr = map + offset;
	int equal = get_equal(ptr, max);
	if (equal > 2) {
	    print_hex(fp, 0x80 | equal, &size);
	    print_hex(fp, *ptr, &size);
	    offset += equal;
	}
	else {
	    int diffs = get_diffs(ptr, max);
	    print_hex(fp, diffs, &size);
	    for (int i = 0; i < diffs; i++) {
		print_hex(fp, ptr[i], &size);
	    }
	    offset += diffs;
	}
    }
    if (size & 7) fprintf(fp, "\n");
}

static void process_tiles(unsigned char *buf) {
    char name[256];
    replace_ext(name, "hdr");
    FILE *fp = fopen(name, "w");
    name[strlen(name) - 4] = 0;

    unsigned char map[header.w * header.h];

    fprintf(fp, "static const byte %s_data[] = {\n", name);
    compress_map(fp, map, generate_data_map(buf, map));
    fprintf(fp, "};\n");

    fprintf(fp, "static const byte %s_attr[] = {\n", name);
    compress_map(fp, map, generate_attr_map(buf, map));
    fprintf(fp, "};\n");

    fclose(fp);
}

static void save_tiles(unsigned char *buf) {
    load_tileset();
    process_tiles(buf);
    save_tileset();
}

static float frequencies[] = {
    16.35, 32.70,  65.41, 130.81, 261.63, 523.25, 1046.50, 2093.00, 4186.00,
    17.32, 34.65,  69.30, 138.59, 277.18, 554.37, 1108.73, 2217.46, 4434.92,
    18.35, 36.71,  73.42, 146.83, 293.66, 587.33, 1174.66, 2349.32, 4698.63,
    19.45, 38.89,  77.78, 155.56, 311.13, 622.25, 1244.51, 2489.00, 4978.00,
    20.60, 41.20,  82.41, 164.81, 329.63, 659.25, 1318.51, 2637.00, 5274.00,
    21.83, 43.65,  87.31, 174.61, 349.23, 698.46, 1396.91, 2793.83, 5587.65,
    23.12, 46.25,  92.50, 185.00, 369.99, 739.99, 1479.98, 2959.96, 5919.91,
    24.50, 49.00,  98.00, 196.00, 392.00, 783.99, 1567.98, 3135.96, 6271.93,
    25.96, 51.91, 103.83, 207.65, 415.30, 830.61, 1661.22, 3322.44, 6644.88,
    27.50, 55.00, 110.00, 220.00, 440.00, 880.00, 1760.00, 3520.00, 7040.00,
    29.14, 58.27, 116.54, 233.08, 466.16, 932.33, 1864.66, 3729.31, 7458.62,
    30.87, 61.74, 123.47, 246.94, 493.88, 987.77, 1975.53, 3951.00, 7902.13,
};

static unsigned get_note(int note, int octave, int length) {
    const float cpu = 1789773.0 / 16; /* 1662607.0 for PAL */
    unsigned period = roundf(cpu / frequencies[9 * note + octave] - 1);
    period = ((period & 0xff) << 8) | ((period >> 8) & 0x7);

    unsigned fade = (length >> 8) & 0xff;
    if (!fade) printf("#error BAD ENVELOPE\n");

    return (fade << 24) | (period << 8) | (length & 0xff);
}

#define FADE(f, l)	((l & 0xff) | (f << 8))
#define NOTE(n, o, l)	((l & 0xffff) | (n << 16) | (o << 24))

#define L1		96
#define L2		48
#define L4		24
#define L4t		L2 / 3
#define L8		12
#define L8t		L4 / 3

/* staccato */
#define Ls4		FADE(1, L4)
#define Ls4t		FADE(1, L4t)
#define Ls4p		FADE(1, L4 + L8)
#define Ls8		FADE(1, L8)

/* slurs */
#define Lr2		FADE(2, L2)
#define Lr4		FADE(2, L4)
#define Lr4t		FADE(2, L4t)
#define Lr8		FADE(2, L8)
#define Lr8t		FADE(2, L8t)

/* quiet staccato */
#define Ls4q		FADE(3, L4)

/* quiet slur */
#define Lr4q		FADE(4, L4)
#define Lr8q		FADE(4, L8)

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

static unsigned hb_bass_0[] = {
    D(2, Ls4q), P(L8), A(2, Lr8q), F(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_1[] = {
    D(2, Ls4q), P(L8), As(2, Lr8q), G(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_2[] = {
    D(2, Ls4q), P(L8), A(2, Lr8q), Fs(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_3[] = {
    D(2, Ls4q), P(L8), A(2, Lr8q), G(3, Ls4q), A(2, Ls4q), END
};

static unsigned hb_bass_4[] = {
    D(2, Ls4q), P(L4), A(3, Ls4q), G(3, Lr4q), END
};

static unsigned hb_bass_5[] = {
    Fs(3, Ls4q), P(L8), A(2, Lr8q), D(2, Ls4q), P(L4), END
};

static void *habanera_bass[] = {
    hb_bass_0, hb_bass_0, hb_bass_0, hb_bass_0, hb_bass_0,
    hb_bass_0, hb_bass_0, hb_bass_1, hb_bass_1, hb_bass_1,
    hb_bass_1, hb_bass_0, hb_bass_0, hb_bass_1, hb_bass_1,
    hb_bass_1, hb_bass_1, hb_bass_1, hb_bass_1, hb_bass_2,
    hb_bass_2, hb_bass_2, hb_bass_2, hb_bass_3, hb_bass_3,
    hb_bass_3, hb_bass_3, hb_bass_2, hb_bass_2, hb_bass_2, hb_bass_2,
    hb_bass_3, hb_bass_3, hb_bass_3, hb_bass_3, hb_bass_2, hb_bass_2,
    hb_bass_2, hb_bass_2, hb_bass_3, hb_bass_3, hb_bass_3,
    hb_bass_4, hb_bass_5,
    NULL,
};

static unsigned hb_high_0[] = {
    P(L1), END
};

static unsigned hb_high_1[] = {
    P(L2), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_2[] = {
    Cs(5, Ls4t), Cs(5, Ls4t), Cs(5, Ls4t), B(4, Ls4), As(4, Ls4), END
};

static unsigned hb_high_3[] = {
    A(4, Ls4p), A(4, Ls8), Gs(4, Ls4), G(4, Ls4), END
};

static unsigned hb_high_4[] = {
    F(4, Lr8t), G(4, Lr8t), F(4, Lr8t), E(4, Lr8), F(4, Lr8),
    G(4, Ls4), F(4, Ls4), END
};

static unsigned hb_high_5[] = {
    E(4, Ls4), P(L4), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_6[] = {
    A(4, Ls4p), A(4, Ls8), G(4, Ls4), F(4, Ls4), END
};

static unsigned hb_high_7[] = {
    E(4, Lr8t), F(4, Lr8t), E(4, Lr8t), D(4, Lr8), E(4, Lr8),
    F(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_8[] = {
    D(4, Lr2), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_9[] = {
    D(4, Ls4), P(L4), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_A[] = {
    C(5, Ls4t), C(5, Ls4t), C(5, Ls4t), B(4, Ls4), As(4, Ls4), END
};

static unsigned hb_high_B[] = {
    A(4, Ls4p), A(4, Lr8), Gs(4, Ls4), G(4, Ls4), END
};

static unsigned hb_high_C[] = {
    Fs(4, Lr8t), G(4, Lr8t), Fs(4, Lr8t), E(4, Lr8), Fs(4, Lr8),
    G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_D[] = {
    E(4, Ls4), P(L4), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_E[] = {
    A(4, Ls4p), A(4, Lr8), G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_F[] = {
    E(4, Lr8t), Fs(4, Lr8t), E(4, Lr8t), D(4, Lr8), E(4, Lr8),
    Fs(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_G[] = {
    D(4, Ls4), P(L8), A(4, Lr8), D(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_H[] = {
    Fs(4, Ls4p), A(4, Lr8), Fs(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_I[] = {
    D(4, Ls4p), E(4, Lr8), Fs(4, Ls4), G(4, Ls4), END
};

static unsigned hb_high_J[] = {
    A(4, Ls8), A(4, Ls8), A(4, Ls8), A(4, Ls8), B(4, Ls4), A(4, Ls4), END
};

static unsigned hb_high_K[] = {
    G(4, Ls4), P(L8), B(4, Lr8), E(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_L[] = {
    G(4, Ls4p), B(4, Lr8), G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_M[] = {
    E(4, Ls4p), Fs(4, Lr8), G(4, Ls4), A(4, Ls4), END
};

static unsigned hb_high_N[] = {
    B(4, Ls8), B(4, Ls8), B(4, Ls8), B(4, Ls8), Cs(5, Ls4), B(4, Ls4), END
};

static unsigned hb_high_O[] = {
    A(4, Ls4), P(L8), A(4, Lr8), D(5, Ls4), E(5, Ls4), END
};

static unsigned hb_high_P[] = {
    Fs(5, Ls4), P(L8), A(4, Lr8), Fs(4, Ls4), E(4, Ls4), END
};

static unsigned hb_high_Q[] = {
    A(4, Ls8), A(4, Ls8), A(4, Ls8), A(4, Ls8), D(5, Ls4), Cs(5, Ls4), END
};

static unsigned hb_high_R[] = {
    A(4, Ls4), P(L8), B(4, Lr8), E(5, Ls4), Fs(5, Ls4), END
};

static unsigned hb_high_S[] = {
    G(5, Ls4), P(L8), B(4, Lr8), G(4, Ls4), Fs(4, Ls4), END
};

static unsigned hb_high_T[] = {
    Cs(5, Lr8), B(4, Lr8), Gs(4, Lr8), A(4, Lr8), Fs(5, Lr4),
    E(5, Lr8t), Fs(5, Lr8t), E(5, Lr8t), END
};

static unsigned hb_high_U[] = {
    D(5, Ls4), P(L8), A(5, Lr8), D(6, Ls4), P(L4), END
};

static void *habanera_high[] = {
    hb_high_0, hb_high_0, hb_high_0, hb_high_1, hb_high_2,
    hb_high_3, hb_high_4, hb_high_5, hb_high_2, hb_high_6,
    hb_high_7, hb_high_8, hb_high_2, hb_high_3, hb_high_4,
    hb_high_5, hb_high_2, hb_high_6, hb_high_7, hb_high_9,
    hb_high_A, hb_high_B, hb_high_C, hb_high_D, hb_high_A,
    hb_high_E, hb_high_F, hb_high_G, hb_high_H, hb_high_I, hb_high_J,
    hb_high_K, hb_high_L, hb_high_M, hb_high_N, hb_high_O, hb_high_P,
    hb_high_I, hb_high_Q, hb_high_R, hb_high_S, hb_high_M,
    hb_high_T, hb_high_U,
    NULL,
};

static void print_note(unsigned note) {
    unsigned value;
    if ((note >> 24) == 0) {
	value = note & 0xff; /* save pause */
    }
    else {
	value = get_note((note >> 16) & 0xff, note >> 24, note & 0xffff);
    }
    for (int i = 24; i >= 0; i -= 8) {
	printf(" 0x%02x,", (value >> i) & 0xff);
    }
    printf("\n");
}

static void print_bar(unsigned *ptr) {
    printf("static const byte bar_%p[] = {\n", ptr);
    while (*ptr != END) {
	print_note(*ptr++);
    }
    printf(" 0xff,\n");
    printf("};\n");
}

static void print_sheet_bars(void **sheet) {
    while (*sheet != NULL) {
	unsigned *bar = *sheet;
	if (*bar != DONE) {
	    print_bar(bar);
	    *bar = DONE;
	}
	sheet++;
    }
}

static void print_sheet(const char *name, void **sheet) {
    print_sheet_bars(sheet);

    printf("static const byte * const %s[] = {\n", name);
    while (*sheet != NULL) {
	printf(" bar_%p,\n", *sheet);
	sheet++;
    }
    printf(" NULL,\n", *sheet);
    printf("};\n");
}

static int alley_height[] = {
    208, 208, 208, 208, 208, 208, 208
};

#define MAX_FISH 10000

struct Fish {
    int time;
    char type;
};

static char *habanera_fish[] = {
    "_______", "_______", "_______", "21_____", "01212__",            // 00012
    "3432___", "3xx5x43", "232____", "10101__", "2323___",            // 34526
    "2xx4x32", "345____", "43232__", "3234___", "5xx3x43",            // 78234
    "212____", "10101__", "2323___", "4xx6x56", "545____",            // 52679
    "43232__", "1010___", "1xx3x23", "454____", "56565__",            // ABCDA
    "4343___", "4xx6x54", "3454___", "5432___", "1212___", "345656_", // EFGHIJ
    "5434___", "3232___", "1232___", "123454_", "3212___", "3454___", // KLMNOP
    "5654___", "3454___", "5434___", "3212___", "1212___",            // IQRSM
    "34343xx2", "343____",                                            // TU
    NULL,
};

static int output_byte(int count, unsigned char data) {
    printf(" 0x%02x,", data);
    if ((count & 7) == 7) {
	printf("\n");
    }
    return count + 1;
}

#define BEST_SCORE 8

static void print_fish(char *name, struct Fish *map, int count) {
    int time = 0;
    printf("static const byte %s[] = {\n", name);
    for (int i = 0; i < count; i++) {
	int offset = map[i].time - time;
	if (offset > 255 || offset == 0) {
	    printf("#error BAD INTERVAL\n");
	}
	printf(" 0x%02x,", offset);
	printf(" 0x%02x,", map[i].type);
	if ((i & 3) == 3) printf("\n");
	time += offset;
    }
    if ((count & 3) != 0) printf("\n");
    printf(" 0x00, 0xff\n");
    printf("};\n");
}

static int build_fish(void *ptr, char **level, int *height, void **sheet) {
    int time = 0;
    int count = 0;
    struct Fish *map = ptr;
    while (*level && *sheet) {
	int index = 0;
	unsigned *note = *sheet;
	const char *fish = *level;
	while (*note != END) {
	    if (*note >> 24) {
		char type = fish[index];
		if (type >= '0' && type <= '9') {
		    type = type - '0';
		    if (count >= MAX_FISH) {
			printf("#error TOO MUCH FISH\n");
			return MAX_FISH;
		    }
		    map[count].time = time - height[type] + BEST_SCORE;
		    map[count].type = type;
		    count++;
		    index++;
		}
		else if (type == 'x') {
		    index++;
		}
	    }
	    time += *note & 0xff;
	    note++;
	}
	level++;
	sheet++;
    }
    return count;
}

static void print_level(char *name, char **level, int *height, void **sheet) {
    struct Fish map[MAX_FISH];
    print_fish(name, map, build_fish(map, level, height, sheet));
}

static int save_music(void) {
    print_level("habanera_fish", habanera_fish, alley_height, habanera_high);
    print_sheet("habanera_bass", habanera_bass);
    print_sheet("habanera_high", habanera_high);
    return 0;
}

int main(int argc, char **argv) {
    char option;

    if (argc < 3) {
	printf("USAGE: pcx-dump [option] file.pcx\n");
	printf("  -r   reset tiles\n");
	printf("  -t   save tiles\n");
	printf("  -p   pad tiles\n");
	printf("  -s   save sprites\n");
	printf("  -m   print music\n");
	return 0;
    }

    file_name = argv[2];
    option = argv[1][1];

    switch (option) {
    case 'r':
	return reset_tiles();
    case 'p':
	return pad_tiles();
    case 'm':
	return save_music();
    }

    unsigned char *buf = read_pcx(file_name);
    if (buf == NULL) return -ENOENT;

    switch (option) {
    case 't':
	save_tiles(buf);
	break;
    case 's':
	save_sprites(buf);
	break;
    }

    free(buf);
    return 0;
}
