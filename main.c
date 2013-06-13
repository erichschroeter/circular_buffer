#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "circular_buffer.h"

#define BUFFER_SIZE 10

int main(int argc, char **argv)
{
	struct circular_buffer *buf;
	int i, j, ret;
	char data[BUFFER_SIZE], testdata[1000];

	/* populate an array with test data */
	for (i = 0; i < 1000; i++)
		testdata[i] = i + 1;

	/* initialize a circular buffer */
	buf = circular_buffer_create(BUFFER_SIZE);
	if (!buf)
		return -1;

	int slice = 5;
	/* copy the test data to the circular buffer */
	for (i = 0; i < 5; i++) {
		circular_buffer_write(buf, testdata + (i * slice), slice);
		printf("WRITE: ");
		for (j = 0; j < slice; j++)
			printf("%d ", testdata[(i * slice) + j]);
		printf("\n");
	circular_buffer_debug(buf);

		circular_buffer_read(buf, data, slice - 2);
		printf("READ : ");
		for (j = 0; j < slice; j++)
			printf("%d ", data[j]);
		printf("\n");
	circular_buffer_debug(buf);
		printf("\n");
		sleep(1);
	}
	printf("\n");
	circular_buffer_debug(buf);
	circular_buffer_destroy(buf);

	return 0;
}
