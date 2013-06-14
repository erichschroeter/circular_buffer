#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "dbg.h"
#include "circular_buffer.h"

struct circular_buffer *circular_buffer_create(int length)
{
	struct circular_buffer *buffer = calloc(1, sizeof(struct circular_buffer));
	if (!buffer)
		return NULL;

	buffer->length = length;
	buffer->tail = 0;
	buffer->head = 0;
	buffer->buffer = calloc(buffer->length + 1, sizeof(char));

	return buffer;
}

void circular_buffer_destroy(struct circular_buffer *buffer)
{
	if (buffer) {
		free(buffer->buffer);
		free(buffer);
	}
}

int circular_buffer_available_data(struct circular_buffer *buffer)
{
	if (buffer->head == buffer->tail) return 0;
	if (buffer->tail < buffer->head) return buffer->head - buffer->tail;
	return (buffer->length + 1) - buffer->tail + buffer->head;
}

int circular_buffer_available_space(struct circular_buffer *buffer)
{
	return buffer->length - circular_buffer_available_data(buffer);
}

int circular_buffer_read(struct circular_buffer *buffer, char *target, int amount)
{
	int available = circular_buffer_available_data(buffer);
	amount = amount > available ? available : amount;

	if (amount <= 0) return 0;

	if (buffer->tail <= buffer->head) {
		memcpy(target, circular_buffer_starts_at(buffer), amount);
	} else {
		int tail_space = buffer->length - buffer->tail;
		if (tail_space <= amount) {
			memcpy(target, circular_buffer_starts_at(buffer), tail_space);
			memcpy(target + tail_space,
				circular_buffer_starts_at(buffer) + tail_space,
				amount - tail_space);
		} else {
			memcpy(target, circular_buffer_starts_at(buffer), amount);
		}
	}

	buffer->tail = (buffer->tail + amount) % (buffer->length + 1);

	return amount;
}

int circular_buffer_write(struct circular_buffer *buffer, char *data, int amount)
{
	if (amount > circular_buffer_available_space(buffer)) {
		debug("Not enough space: %d request, %d available",
			amount, circular_buffer_available_space(buffer));
		return -1;
	}

	if (buffer->head >= buffer->tail) {
		int head_space = buffer->length - buffer->head;
		if (head_space >= amount) {
			memcpy(circular_buffer_ends_at(buffer), data, amount);
		} else {
			memcpy(circular_buffer_ends_at(buffer), data, head_space);
			memcpy(circular_buffer_ends_at(buffer) + head_space,
				data + head_space, amount - head_space);
		}
	} else {
		memcpy(circular_buffer_ends_at(buffer), data, amount);
	}

	buffer->head = (buffer->head + amount) % (buffer->length + 1);

	return amount;
}

void circular_buffer_debug(struct circular_buffer *buf)
{
	int i;
	printf("{ length='%d' tail='%d' head='%d' available_data='%d' available_space='%d' buffer='",
		buf->length, buf->tail, buf->head, circular_buffer_available_data(buf),
		circular_buffer_available_space(buf));
	for (i = 0; i < buf->length; i++) {
		if (i != 0)
			printf(" ");
		printf("%x", buf->buffer[i]);
	}
	printf("' }\n");
}
