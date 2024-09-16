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

#define DEFAULT_FADE	0xf

static unsigned get_note(int note, int octave, int length) {
    const float cpu = 1789773.0 / 16; /* 1662607.0 for PAL */
    unsigned period = roundf(cpu / frequencies[9 * note + octave] - 1);
    period = ((period & 0xff) << 8) | ((period >> 8) & 0x7) | 0x8;

    unsigned fade = (length >> 8) & 0xff;
    if (fade == 0x00) fade = DEFAULT_FADE;

    return (fade << 24) | (period << 8) | (length & 0xff);
}

#define FADE(f, l)	((l & 0xff) | (f << 8))
#define NOTE(n, o, l)	((l << 16) | (n << 8) | o)

#define L2		120
#define L4		60
#define L8		30
#define L8t		L4 / 3
#define L16		15
#define L16t		L8 / 3

/* staccato */
#define L8s		FADE(5, L8)
#define L8st		FADE(5, L8t)
#define L8sp		FADE(5, L8 + L16)
#define L16s		FADE(5, L16)

#define END		(0xff << 24)
#define DONE		(0xde << 24)
#define P(l)		(0xf0 << 24) | (l & 0xff)
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
    D(2, L8s), P(L16), A(2, L16), F(3, L8s), A(2, L8s), END
};

static unsigned hb_bass_1[] = {
    D(2, L8s), P(L16), As(2, L16), G(3, L8s), A(2, L8s), END
};

static void *habanera_bass[] = {
    hb_bass_0, hb_bass_0, hb_bass_0, hb_bass_0, hb_bass_0,
    hb_bass_0, hb_bass_0, hb_bass_1, hb_bass_1, hb_bass_1,
    hb_bass_1, hb_bass_0, hb_bass_0, hb_bass_1, hb_bass_1,
    NULL,
};

static unsigned hb_high_0[] = {
    P(L2), END
};

static unsigned hb_high_1[] = {
    P(L4), D(5, L8s), Cs(5, L8s), END
};

static unsigned hb_high_2[] = {
    Cs(5, L8st), Cs(5, L8st), Cs(5, L8st), B(4, L8s), As(4, L8s), END
};

static unsigned hb_high_3[] = {
    A(4, L8sp), A(4, L16s), Gs(4, L8s), G(4, L8s), END
};

static unsigned hb_high_4[] = {
    F(4, L16t), G(4, L16t), F(4, L16t), E(4, L16), F(4, L16),
    G(4, L8s), F(4, L8s), END
};

static unsigned hb_high_5[] = {
    E(4, L8s), P(L8), D(5, L8s), Cs(5, L8s), END
};

static unsigned hb_high_6[] = {
    A(4, L8sp), A(4, L16s), G(4, L8s), F(4, L8s), END
};

static unsigned hb_high_7[] = {
    E(4, L16t), F(4, L16t), E(4, L16t), D(4, L16), E(4, L16),
    F(4, L8s), E(4, L8s), END
};

static unsigned hb_high_8[] = {
    D(4, L4), D(5, L8s), Cs(5, L8s), END
};

static void *habanera_high[] = {
    hb_high_0, hb_high_0, hb_high_0, hb_high_1, hb_high_2,
    hb_high_3, hb_high_4, hb_high_5, hb_high_2, hb_high_6,
    hb_high_7, hb_high_8, hb_high_2, hb_high_3, hb_high_4,
    NULL,
};

static void print_note(unsigned note) {
    unsigned value;
    if ((note >> 24) == 0xf0) {
	value = note & 0xff; /* save pause */
    }
    else {
	value = get_note((note >> 8) & 0xff, note & 0xff, note >> 16);
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

static int save_music(void) {
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
