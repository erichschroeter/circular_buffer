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

static void lock(struct circular_buffer *buffer)
{
#ifdef WIN32
	/* TODO support Windows mutex equivalent */
#else
	pthread_mutex_lock(&buffer->mutex);
#endif
}

static void unlock(struct circular_buffer *buffer)
{
#ifdef WIN32
	/* TODO support Windows mutex equivalent */
#else
	pthread_mutex_unlock(&buffer->mutex);
#endif
}

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
	/* TODO support Windows mutex equivalent */
#else /* Unix */
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
	pthread_mutex_init(&buffer->mutex, &attr);
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
	/* Make sure no other threads are using the buffer before destroying it. */
#ifdef WIN32
	/* TODO support Windows mutex equivalent */
#else
	pthread_mutex_destroy(&buffer->mutex);
#endif
	free(buffer->buffer);
	free(buffer);
}

static int _available_data(struct circular_buffer *buffer)
{
	int available = 0;

	if (buffer->head == buffer->tail)
		available = 0;
	else if (buffer->tail < buffer->head)
		available = buffer->head - buffer->tail;
	else
		available = (buffer->length + 1) - buffer->tail + buffer->head;

	return available;
}

static int _available_space(struct circular_buffer *buffer)
{
	return buffer->length - _available_data(buffer);
}

CBAPI int CBCALL cb_available_data(struct circular_buffer *buffer)
{
	int available = 0;

	lock(buffer);

	available = _available_data(buffer);

	unlock(buffer);

	assert(available >= 0);

	return available;
}

CBAPI int CBCALL cb_available_space(struct circular_buffer *buffer)
{
	int available = 0;

	lock(buffer);

	available = _available_space(buffer);

	unlock(buffer);

	assert(available >= 0);

	return available;
}

/**
 * Attempts to read up to `amount` bytes from `buffer` into the buffer
 * starting at `target`.
 *
 * If `amount` is zero or less, zero is returned.
 *
 * @param buffer the buffer to read from
 * @param target location to copy data to
 * @param amount the number of bytes to attempt to copy
 *
 * @return A maximum of `amount` of bytes read into `target`.
 */
CBAPI int CBCALL cb_read(struct circular_buffer *buffer, char *target, int amount)
{
	int available, ret = 0;

	if (amount < 1)
		return 0;

	lock(buffer);

	available = _available_data(buffer);

	if (amount > available) {
		amount = available;
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
	assert(buffer->tail <= (buffer->length + 1));
	assert(buffer->tail >= 0);

out:
	unlock(buffer);

	return amount;
}

CBAPI int CBCALL cb_read_single(struct circular_buffer *buffer, char *target)
{
	return cb_read(buffer, target, 1);
}

CBAPI int CBCALL cb_write(struct circular_buffer *buffer, char *data, int amount)
{
	int ret = 0;

	lock(buffer);

	if (amount > _available_space(buffer)) {
		debug("Not enough space: %d request, %d available",
			amount, _available_space(buffer));
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
	assert(buffer->head <= (buffer->length + 1));
	assert(buffer->head >= 0);

out:
	unlock(buffer);

	return amount;
}

CBAPI void CBCALL cb_debug(struct circular_buffer *buf)
{
	int i;
	lock(buf);
	printf("{ length='%d' tail='%d' head='%d' available_data='%d' available_space='%d' buffer='",
		buf->length, buf->tail, buf->head, _available_data(buf),
		_available_space(buf));
	for (i = 0; i < buf->length + 1; i++) {
		if (i != 0)
			printf(" ");
		if (i == buf->tail)
			printf("tail->");
		if (i == buf->head)
			printf("head->");
		printf("%x", buf->buffer[i] & 0xff);
	}
	unlock(buf);
	printf("' }\n");
}

CBAPI void CBCALL cb_clear(struct circular_buffer *buf)
{
	buf->tail = 0;
	buf->head = 0;
}
