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

static void process_tiles(unsigned char *buf) {
    for (int y = 0; y < header.h; y += 8) {
	for (int x = 0; x < header.w; x += 8) {
	    unsigned char result[16];
	    unsigned char *ptr = buf + y * header.w + x;
	    get_bit_plane(ptr, result + 0, 1);
	    get_bit_plane(ptr, result + 8, 2);

	    int id = look_up_tile(result);
	}
    }
}

static void save_tiles(unsigned char *buf) {
    load_tileset();
    process_tiles(buf);
    save_tileset();
}

int main(int argc, char **argv) {
    char option;

    if (argc < 3) {
	printf("USAGE: pcx-dump [option] file.pcx\n");
	printf("  -r   reset tiles\n");
	printf("  -t   save tiles\n");
	printf("  -p   pad tiles\n");
	printf("  -s   save sprites\n");
	return 0;
    }

    file_name = argv[2];
    option = argv[1][1];

    switch (option) {
    case 'r':
	return reset_tiles();
    case 'p':
	return pad_tiles();
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
