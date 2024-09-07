#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

static char *file_name;

struct Header {
    unsigned short w, h;
} header;

static unsigned char *read_pcx(const char *file) {
    struct stat st;
    int palette_offset = 16;
    if (stat(file, &st) != 0) {
	fprintf(stderr, "ERROR while opening PCX-file \"%s\"\n", file);
	return NULL;
    }
    unsigned char *buf = malloc(st.st_size);
    int in = open(file, O_RDONLY);
    read(in, buf, st.st_size);
    close(in);

    header.w = (* (unsigned short *) (buf + 0x8)) + 1;
    header.h = (* (unsigned short *) (buf + 0xa)) + 1;
    if (buf[3] == 8) palette_offset = st.st_size - 768;
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

int main(int argc, char **argv) {
    if (argc < 3) {
	printf("USAGE: pcx-dump [option] file.pcx\n");
	printf("  -t   save tiles\n");
	printf("  -l   save layout\n");
	return 0;
    }

    file_name = argv[2];

    unsigned char *buf = read_pcx(file_name);
    if (buf == NULL) return -ENOENT;
    free(buf);

    return 0;
}
