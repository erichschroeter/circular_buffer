#include <stdio.h>
#include <string.h>

#include "circular_buffer.h"

#define DATA_SIZE 256
#define BUFFER_SIZE 10

int main(int argc, char **argv)
{
	int ret, i;
	char data[DATA_SIZE + 1];
	struct circular_buffer *b = 0;
	
	b = circular_buffer_create(BUFFER_SIZE);

	if (!b) return -1;

	memset(data, 0, sizeof(data));

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			/*printf("argv[%d]='%s'\n", i, argv[i]);*/
retry:
			ret = circular_buffer_write(b, argv[i], strlen(argv[i]));
				printf("write ret=%d\targv[%d]=%s\n", ret, i, argv[i]);
			if (ret == -1) {
				ret = circular_buffer_read(b, data, sizeof(data));
				data[ret] = 0;
				printf("read  ret=%d\n", ret);
				fprintf(stderr, "buffer dump='%s'\n", data);
				memset(data, 0, sizeof(data));
				goto retry;
				/*fprintf(stderr,*/
				/*"Not enough space to write '%s' in buffer of size %d bytes\n",*/
				/*argv[i], b->length);*/
				/*break;*/
			}
		}
	} else {
		fprintf(stderr, "Nothing to write to buffer.\n");
		return -1;
	}

	circular_buffer_read(b, data, sizeof(data));
	data[DATA_SIZE] = 0;
	printf("buffer contents='%s'\n", data);

	circular_buffer_destroy(b);

	return 0;
}
