#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#include "circular_buffer.h"

#define DATA_SIZE 256

static struct option _options[] = {
	 { "buffer", no_argument, 0, 'b' },
};

int main(int argc, char **argv)
{
	int ret, i, c, buffer_size = 10;
	char data[DATA_SIZE + 1];
	struct circular_buffer *b = 0;
	
	while ((c = getopt_long(argc, argv, "b:", _options, 0)) != -1) {
		switch (c) {
		case 'b':
			buffer_size = strtol(optarg, NULL, 10);
			break;
		}
	}

	b = circular_buffer_create(buffer_size);

	if (!b) return -1;

	memset(data, 0, sizeof(data));

	if (argc - optind < 1) {
		fprintf(stderr, "Nothing to write to buffer.\n");
		return -1;
	}

	for (i = optind; i < argc; i++) {
		int written = 0;
		do {
			if (circular_buffer_full(b)) {
				ret = circular_buffer_read(b, data, sizeof(data));
				data[ret] = 0;
				fprintf(stderr, "read\t'%s'\n", data);
				memset(data, 0, sizeof(data));
			}

			ret = circular_buffer_write(b, argv[i] + written, circular_buffer_available_space(b));
			/* If write was successful, inform user what was written. */
			char copy[ret + 1];
			snprintf(copy, ret + 1, "%s", argv[i] + written);
			if (ret > 0) printf("write\t'%s'\n", copy);
			written += ret;
		} while (written <= strlen(argv[i]));
	}

	circular_buffer_read(b, data, sizeof(data));
	data[DATA_SIZE] = 0;

	circular_buffer_destroy(b);

	return 0;
}
