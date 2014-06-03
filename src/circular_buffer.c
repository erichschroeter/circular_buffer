#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#include "circular_buffer.h"

#ifdef DEBUG
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

CBAPI struct circular_buffer * CBCALL cb_create(int length)
{
	struct circular_buffer *buffer = calloc(1, sizeof(struct circular_buffer));
	if (!buffer)
		return NULL;

	buffer->length = length;
	buffer->tail = 0;
	buffer->head = 0;
	buffer->buffer = calloc(buffer->length + 1, sizeof(char));
	if (!buffer->buffer) goto fail;

#ifdef WIN32
#else
	int ret = sem_init(&buffer->mutex, 0, 1);
	if (ret != 0) goto fail_mutex;
#endif

	return buffer;
fail_mutex:
	free(buffer->buffer);
fail:
	free(buffer);
	return NULL;
}

CBAPI void CBCALL cb_destroy(struct circular_buffer *buffer)
{
	int ret;

	/* Make sure no other threads are using the buffer before destroying it. */
#ifdef WIN32
#else
	ret = sem_wait(&buffer->mutex);
	if (ret)
		return;
#endif
	free(buffer->buffer);
	free(buffer);
}

CBAPI int CBCALL cb_available_data(struct circular_buffer *buffer)
{
	int available = 0;

	if (buffer->head == buffer->tail)
		available = 0;
	else if (buffer->tail < buffer->head)
		available = buffer->head - buffer->tail;
	else
		available = (buffer->length + 1) - buffer->tail + buffer->head;

	assert(available >= 0);

	return available;
}

CBAPI int CBCALL cb_available_space(struct circular_buffer *buffer)
{
	int available = buffer->length - cb_available_data(buffer);
	assert(available >= 0);
	return available;
}

CBAPI int CBCALL cb_read(struct circular_buffer *buffer, char *target, int amount)
{
	int available, ret = 0;

#ifdef WIN32
#else
	ret = sem_wait(&buffer->mutex);
	if (ret) {
		amount = 0;
		goto out;
	}
#endif

	available = cb_available_data(buffer);
	amount = amount > available ? available : amount;

	if (amount <= 0) {
		amount = 0;
		goto out;
	}

	if (buffer->tail <= buffer->head) {
		memcpy(target, cb_starts_at(buffer), amount);
	} else {
		int tail_space = (buffer->length + 1) - buffer->tail;
		assert(tail_space >= 0);
		if (tail_space <= amount) {
			memcpy(target, cb_starts_at(buffer), tail_space);
			memcpy(target + tail_space, buffer->buffer, amount - tail_space);
		} else {
			memcpy(target, cb_starts_at(buffer), amount);
		}
	}

	buffer->tail = (buffer->tail + amount) % (buffer->length + 1);

out:
#ifdef WIN32
#else
	sem_post(&buffer->mutex);
#endif

	return amount;
}

CBAPI int CBCALL cb_read_single(struct circular_buffer *buffer, char *target)
{
	int ret;
	char tmp[1];

	ret = cb_read(buffer, tmp, 1);
	*target = tmp[0];

	return ret;
}

CBAPI int CBCALL cb_write(struct circular_buffer *buffer, char *data, int amount)
{
	int ret = 0;

#ifdef WIN32
#else
	ret = sem_wait(&buffer->mutex);
	if (ret) {
		amount = 0;
		goto out;
	}
#endif

	if (amount > cb_available_space(buffer)) {
		debug("Not enough space: %d request, %d available",
			amount, cb_available_space(buffer));
		amount = -1;
		goto out;
	}

	if (buffer->head >= buffer->tail) {
		int head_space = (buffer->length + 1) - buffer->head;
		assert(head_space >= 0);
		if (head_space >= amount) {
			memcpy(cb_ends_at(buffer), data, amount);
		} else {
			memcpy(cb_ends_at(buffer), data, head_space);
			memcpy(buffer->buffer, data + head_space, amount - head_space);
		}
	} else {
		memcpy(cb_ends_at(buffer), data, amount);
	}

	buffer->head = (buffer->head + amount) % (buffer->length + 1);

out:
#ifdef WIN32
#else
	sem_post(&buffer->mutex);
#endif

	return amount;
}

CBAPI void CBCALL cb_debug(struct circular_buffer *buf)
{
	int i;
	printf("{ length='%d' tail='%d' head='%d' available_data='%d' available_space='%d' buffer='",
		buf->length, buf->tail, buf->head, cb_available_data(buf),
		cb_available_space(buf));
	for (i = 0; i < buf->length + 1; i++) {
		if (i != 0)
			printf(" ");
		if (i == buf->tail)
			printf("tail->");
		if (i == buf->head)
			printf("head->");
		printf("%x", buf->buffer[i] & 0xff);
	}
	printf("' }\n");
}

CBAPI void CBCALL cb_clear(struct circular_buffer *buf)
{
	buf->tail = 0;
	buf->head = 0;
}
