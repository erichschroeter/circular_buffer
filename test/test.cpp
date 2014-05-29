#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <circular_buffer.h>

#ifdef _WIN32
#define snprintf _snprintf_s
#define sprintf sprintf_s
#define strcat strcat_s
#endif

static struct circular_buffer* incrementing_buffer(int size)
{
	int i;
	char data[size];
	struct circular_buffer *buffer = cb_create(size);

	REQUIRE(buffer != 0);

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	cb_write(buffer, data, sizeof(data));

	return buffer;
}

TEST_CASE("Circular buffer full function", "[full][utility]")
{
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(10);
	empty = cb_create(20);

	REQUIRE(cb_full(incrementing) == 1);
	REQUIRE(cb_full(empty) == 0);
}

TEST_CASE("Circular buffer empty function", "[empty][utility]")
{
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(10);
	empty = cb_create(20);

	REQUIRE(cb_empty(incrementing) == 0);
	REQUIRE(cb_empty(empty) == 1);
}

TEST_CASE("Circular buffer starts at function", "[utility]")
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(SIZE);

	/* Verify the data starts at the beginning of a full buffer. */
	REQUIRE(cb_starts_at(incrementing) == incrementing->buffer);
	cb_read(incrementing, buffer, READ_SIZE);
	/* Verify the data starts at the correct offset from the beginning of the buffer. */
	REQUIRE(cb_starts_at(incrementing) == incrementing->buffer + READ_SIZE);

	empty = cb_create(20);

	/* Verify the data starts at the beginning of an empty buffer. */
	REQUIRE(cb_starts_at(empty) == empty->buffer);
	cb_read(empty, buffer, READ_SIZE);
	/* Verify the data start position has not changed after reading an empty buffer. */
	REQUIRE(cb_starts_at(empty) == empty->buffer);
}

TEST_CASE("Circular buffer ends at function", "[utility]")
{
	char testdata[] = "test data";
	const unsigned int SIZE = sizeof(testdata);
	char buffer[SIZE];
	struct circular_buffer *full, *empty;

	full = incrementing_buffer(SIZE);

	/* Verify the data ends at the end of a full buffer. */
	REQUIRE(cb_ends_at(full) == full->buffer + full->length);
	/* Make some room to write some more. */
	cb_read(full, buffer, SIZE);
	/* Verify the data end position has not changed from reading. */
	REQUIRE(cb_ends_at(full) == full->buffer + full->length);
	cb_write(full, testdata, SIZE);
	/*
	 * Verify the data ends at the correct offset from the beginning of the buffer.
	 * Since this was a full buffer we need to take into account the head wrapping. The
	 * -1 exists because the actual size of the buffer is length + 1.
	 */
	REQUIRE(cb_ends_at(full) == (full->buffer + SIZE - 1));

	/* Create an empty buffer big enough to fit the test data. */
	empty = cb_create(SIZE + 1);

	/* Verify the data ends at the beginning of an empty buffer. */
	REQUIRE(cb_starts_at(empty) == empty->buffer);
	cb_read(empty, buffer, SIZE);
	/* Verify the data end position has not changed after reading an empty buffer. */
	REQUIRE(cb_starts_at(empty) == empty->buffer);
	cb_write(empty, testdata, SIZE);
	/*
	 * Verify the data ends at the correct offset from the beginning of the buffer.
	 * We don't have to take into account the -1 since we haven't wrapped this buffer.
	 */
	REQUIRE(cb_ends_at(empty) == empty->buffer + SIZE);
}

TEST_CASE("Circular buffer available data function", "[utility]")
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(SIZE);
	empty = cb_create(20);

	/* Verify a full buffer has the correct amount of data. */
	REQUIRE(cb_available_data(incrementing) == SIZE);
	cb_read(incrementing, buffer, READ_SIZE);
	/* Verify the buffer has reduced the available data by the amount read. */
	REQUIRE(cb_available_data(incrementing) == SIZE - READ_SIZE);

	/* Verify an empty buffer has no data available. */
	REQUIRE(cb_available_data(empty) == 0);
	cb_read(empty, buffer, READ_SIZE);
	/* Verify that no data is still available. */
	REQUIRE(cb_available_data(empty) == 0);
}

TEST_CASE("Circular buffer available space function", "[utility]")
{
	const unsigned int SIZE = 10, READ_SIZE = 5, WRITE_SIZE = 5;
	char buffer[SIZE], data[SIZE];
	struct circular_buffer *incrementing, *empty;
	int i;

	incrementing = incrementing_buffer(SIZE);
	empty = cb_create(SIZE);

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	/* Verify there is no space available in a full buffer. */
	REQUIRE(cb_available_space(incrementing) == 0);
	cb_read(incrementing, buffer, READ_SIZE);
	/* Verify the space available is the amount just read of a full buffer. */
	REQUIRE(cb_available_space(incrementing) == READ_SIZE);

	/* Verify the space available is the size of an empty buffer. */
	REQUIRE(cb_available_space(empty) == SIZE);
	cb_read(empty, buffer, READ_SIZE);
	/* Verify that reading doesn't affect the space available of an empty buffer. */
	REQUIRE(cb_available_space(empty) == SIZE);
	cb_write(empty, data, WRITE_SIZE);
	/* Verify the space available is reduced by the amount written. */
	REQUIRE(cb_available_space(empty) == SIZE - WRITE_SIZE);
}

TEST_CASE("Circular buffer clear function", "[utility]")
{
	const unsigned int FULL_SIZE = 10, EMPTY_SIZE = 20;
	struct circular_buffer *full, *empty;

	full = incrementing_buffer(FULL_SIZE);
	empty = cb_create(EMPTY_SIZE);

	cb_clear(full);

	REQUIRE(cb_available_data(full) == 0);
	REQUIRE(cb_available_space(full) == FULL_SIZE);
	REQUIRE(cb_full(full) == 0);
	REQUIRE(cb_empty(full) == 1);

	cb_clear(empty);

	REQUIRE(cb_available_data(empty) == 0);
	REQUIRE(cb_available_space(empty) == EMPTY_SIZE);
	REQUIRE(cb_full(empty) == 0);
	REQUIRE(cb_empty(empty) == 1);
}

TEST_CASE("Circular buffer read all", "[read]")
{
	const unsigned int SIZE = 10;
	char buffer[SIZE];
	struct circular_buffer *incrementing;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);

	REQUIRE(cb_available_data(incrementing) == incrementing->length);
	ret = cb_read(incrementing, buffer, sizeof(buffer));
	REQUIRE(ret == sizeof(buffer));
	REQUIRE(cb_available_data(incrementing) == 0);
	for (i = 0; i < sizeof(buffer); i++) {
		if (buffer[i] != i) {
			FAIL("Data read from incrementing is incorrect.");
			break;
		}
	}
}

TEST_CASE("Circular buffer read none", "[read]")
{
	const unsigned int SIZE = 10;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;
	int ret;

	incrementing = incrementing_buffer(SIZE);
	empty = cb_create(20);

	REQUIRE(cb_available_data(incrementing) == incrementing->length);
	ret = cb_read(incrementing, buffer, 0);
	REQUIRE(ret == 0);
	REQUIRE(cb_available_data(incrementing) == incrementing->length);

	REQUIRE(cb_available_data(empty) == 0);
	ret = cb_read(empty, buffer, 0);
	REQUIRE(ret == 0);
	REQUIRE(cb_available_data(empty) == 0);
}

TEST_CASE("Circular buffer read too much", "[read]")
{
	const unsigned int SIZE = 10;
	char buffer[SIZE + 1];
	struct circular_buffer *incrementing, *empty;
	int ret;

	incrementing = incrementing_buffer(SIZE);
	empty = cb_create(20);

	/*
	 * circular_buffer_read should only return up to the amount of available data. This
	 * means it should return at most the amount specified, but could be less if the amount
	 * specified was more than was available.
	 */
	REQUIRE(cb_available_data(incrementing) == incrementing->length);
	ret = cb_read(incrementing, buffer, sizeof(buffer));
	REQUIRE(ret == SIZE);
	REQUIRE(cb_available_data(incrementing) == 0);

	REQUIRE(cb_available_data(empty) == 0);
	ret = cb_read(empty, buffer, 1);
	REQUIRE(ret == 0);
	REQUIRE(cb_available_data(empty) == 0);
}

TEST_CASE("Circular buffer write all", "[write]")
{
	const unsigned int SIZE = 10;
	char data[SIZE];
	struct circular_buffer *empty;
	int i, ret;

	empty = cb_create(SIZE);

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	REQUIRE(cb_available_data(empty) == 0);
	ret = cb_write(empty, data, SIZE);
	REQUIRE(ret == SIZE);
	REQUIRE(cb_available_data(empty) == SIZE);
	for (i = 0; i < empty->length; i++) {
		if (empty->buffer[i] != i) {
			FAIL("Data written to empty buffer is incorrect.");
			break;
		}
	}
}

TEST_CASE("Circular buffer write none", "[write]")
{
	const unsigned int SIZE = 10;
	struct circular_buffer *empty;
	int ret;
	char data[10];

	empty = cb_create(SIZE);

	ret = cb_write(empty, data, 0);

	REQUIRE(ret == 0);
	REQUIRE(cb_available_data(empty) == 0);
	REQUIRE(cb_available_space(empty) == SIZE);
}

TEST_CASE("Circular buffer write too much", "[write]")
{
	const unsigned int SIZE = 10;
	char data[SIZE + 1];
	struct circular_buffer *empty;
	int i, ret;

	empty = cb_create(SIZE);

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	REQUIRE(cb_available_data(empty) == 0);
	ret = cb_write(empty, data, sizeof(data));
	REQUIRE(ret == -1);
	REQUIRE(cb_available_data(empty) == 0);
	for (i = 0; i < empty->length; i++) {
		if (empty->buffer[i] != 0) {
			FAIL("The buffer was modified when it shouldn't have been.");
			break;
		}
	}
}

TEST_CASE("Circular buffer head wraps", "[read][write]")
{
	const unsigned int SIZE = 10, READ_SIZE = 5, WRITE_SIZE = 5;
	char data[SIZE];
	struct circular_buffer *incrementing;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	/* Attempt to write to a full buffer and expect nothing to change. */
	REQUIRE(incrementing->head == incrementing->length);
	REQUIRE(incrementing->tail == 0);
	ret = cb_write(incrementing, data, WRITE_SIZE);
	REQUIRE(ret == -1);
	REQUIRE(incrementing->head == incrementing->length);
	REQUIRE(incrementing->tail == 0);
	/* Now read some. */
	ret = cb_read(incrementing, data, READ_SIZE);
	REQUIRE(ret == READ_SIZE);
	REQUIRE(incrementing->head == incrementing->length);
	REQUIRE(incrementing->tail == READ_SIZE);
	/* Now write and verify head has wrapped. */
	ret = cb_write(incrementing, data, WRITE_SIZE);
	REQUIRE(ret == WRITE_SIZE);
	/* head should be 1 less than WRITE_SIZE since the actual buffer is length + 1. */
	REQUIRE(incrementing->head == WRITE_SIZE - 1);
	REQUIRE(incrementing->tail == READ_SIZE);
}

TEST_CASE("Circular buffer tail wraps", "[read][write]")
{
	const unsigned int SIZE = 10, READ_SIZE = 4, WRITE_SIZE = 5;
	char data[SIZE];
	struct circular_buffer *incrementing;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	/* Attempt to write to a full buffer and expect nothing to change. */
	REQUIRE(incrementing->head == incrementing->length);
	REQUIRE(incrementing->tail == 0);
	ret = cb_write(incrementing, data, WRITE_SIZE);
	REQUIRE(ret == -1);
	REQUIRE(incrementing->head == incrementing->length);
	REQUIRE(incrementing->tail == 0);
	/* Now read all data. */
	ret = cb_read(incrementing, data, incrementing->length);
	REQUIRE(ret == incrementing->length);
	REQUIRE(incrementing->head == incrementing->length);
	REQUIRE(incrementing->tail == incrementing->length);
	/* Now fill the buffer up again. */
	ret = cb_write(incrementing, data, WRITE_SIZE);
	REQUIRE(ret == WRITE_SIZE);
	/* head should be 1 less than WRITE_SIZE since the actual buffer is length + 1. */
	REQUIRE(incrementing->head == WRITE_SIZE - 1);
	REQUIRE(incrementing->tail == incrementing->length);
	/* Now read and verify tail has wrapped. */
	ret = cb_read(incrementing, data, READ_SIZE);
	REQUIRE(ret == READ_SIZE);
	REQUIRE(incrementing->head == WRITE_SIZE - 1);
	REQUIRE(incrementing->tail == READ_SIZE - 1);
}

TEST_CASE("Circular buffer validate data", "[read][write]")
{
	char data[] = "this is a test", validate[sizeof(data)];
	struct circular_buffer *buffer;
	int i, ret;

	buffer = cb_create(sizeof(data));

	/* Write the test data to the buffer and validate it is actually loaded. */
	ret = cb_write(buffer, data, sizeof(data));
	REQUIRE(ret == sizeof(data));
	for (i = 0; i < sizeof(data); i++) {
		if (buffer->buffer[i] != data[i]) {
			FAIL("Data written to buffer is not valid.");
			break;
		}
	}

	/* Read the test data and validate that it was actually copied. */
	ret = cb_read(buffer, validate, sizeof(validate));
	REQUIRE(ret == sizeof(validate));
	for (i = 0; i < sizeof(validate); i++) {
		if (validate[i] != data[i]) {
			FAIL("Data read from buffer is not valid.");
			break;
		}
	}
}

TEST_CASE("Circular buffer wrapping boundary", "[read][write]")
{
	char data[] = {1,2,3,4}, data2[] = {5,6}, validate[sizeof(data)];
	struct circular_buffer *buffer;
	int i, ret;

	buffer = cb_create(sizeof(data));

	/* Write the test data to the buffer and validate it is actually loaded. */
	ret = cb_write(buffer, data, sizeof(data));
	REQUIRE(ret == sizeof(data));
	for (i = 0; i < sizeof(data); i++) {
		if (buffer->buffer[i] != data[i]) {
			FAIL("Data written to buffer is not valid.");
			break;
		}
	}

	/*
	 * At this point the buffer is full. Now we need to create the condition
	 * where tail is located at the very last position before wrapping and
	 * then read the buffer.
	 */
	ret = cb_read(buffer, validate, 4);
	REQUIRE(ret == 4);
	ret = cb_write(buffer, data2, 2);
	REQUIRE(ret == 2);
	/* Verify that a read that wraps has valid data. */
	ret = cb_read(buffer, validate, 2);
	REQUIRE(ret == 2);
	for (i = 0; i < 2; i++) {
		if (validate[i] != data2[i]) {
			FAIL("Data read from buffer is not valid.");
			break;
		}
	}
}

static volatile int _running = 0;
static void* cb_write_thread(void *circular_buffer)
{
	int i = 0, ret;
	struct circular_buffer *buffer = (struct circular_buffer*) circular_buffer;
	char data[4];

	while (_running) {
		data[0] = (i >> 24) & 0xff;
		data[1] = (i >> 16) & 0xff;
		data[2] = (i >> 8) & 0xff;
		data[3] = i & 0xff;
retry:
		ret = cb_write(buffer, data, 4);
		/*
		 * In case we stop the thread from outside, we don't want to get
		 * stuck in an infinite loop. So, check that we are still running.
		 */
		if (ret < 0 && _running) { usleep(10); goto retry; }
		i++;
	}
	return 0;
}

TEST_CASE("Circular buffer multithreading", "[read][write][multithread]")
{
	int i, ret, value;
	struct circular_buffer *buffer;
	pthread_t thread;
	const unsigned int VERIFY_NUM = 100000;
	char data[4];

	buffer = cb_create(12);
	REQUIRE(buffer != 0);

	_running = 1;
	ret = pthread_create(&thread, NULL, &cb_write_thread, buffer);
	REQUIRE(ret == 0);

	/*
	 * We are testing that each value we read from the buffer is incremented
	 * by one. This will verify that there is no jumping around or overwriting
	 * in the buffer when reading/writing in multithreaded environment.
	 */
	for (i = 0; i < VERIFY_NUM; i++) {
retry:
		ret = cb_read(buffer, data, 4);
		/* In case the write thread hasn't kept up keep trying to read. */
		if (ret < 1) { goto retry; }
		value = ((data[0] << 24) & 0xff000000) |
			((data[1] << 16) & 0xff0000) |
			((data[2] << 8) & 0xff00) |
			(data[3] & 0xff);
		if (value != i) {
			_running = 0;
			FAIL("Multithreaded data is not syncronized.");
			break;
		}
	}

	pthread_cancel(thread);
	cb_destroy(buffer);
}
