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

	if (argc - optind > 1) {
		for (i = optind; i < argc; i++) {
retry:
			ret = circular_buffer_write(b, argv[i], strlen(argv[i]));
			if (ret > 0) printf("write:'%s'\n", argv[i]);
			if (ret == -1) {
				ret = circular_buffer_read(b, data, sizeof(data));
				data[ret] = 0;
				fprintf(stderr, "read:'%s'\n", data);
				memset(data, 0, sizeof(data));
				goto retry;
			}
		}
	} else {
		fprintf(stderr, "Nothing to write to buffer.\n");
		return -1;
	}

	circular_buffer_read(b, data, sizeof(data));
	data[DATA_SIZE] = 0;

	circular_buffer_destroy(b);

	return 0;
}
