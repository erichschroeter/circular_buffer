#include <stdlib.h>

#include <CUnit/Basic.h>

#include "circular_buffer.h"

static struct circular_buffer* incrementing_buffer(int size)
{
	int i;
	char data[size];
	struct circular_buffer *buffer = circular_buffer_create(size);

	CU_ASSERT(buffer != NULL);

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	circular_buffer_write(buffer, data, sizeof(data));

	return buffer;
}

void test_circular_buffer_full(void)
{
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(10);
	empty = circular_buffer_create(20);

	CU_ASSERT_TRUE(circular_buffer_full(incrementing));
	CU_ASSERT_FALSE(circular_buffer_full(empty));
}

void test_circular_buffer_empty(void)
{
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(10);
	empty = circular_buffer_create(20);

	CU_ASSERT_FALSE(circular_buffer_empty(incrementing));
	CU_ASSERT_TRUE(circular_buffer_empty(empty));
}

void test_circular_buffer_starts_at(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(20);

	CU_ASSERT(circular_buffer_starts_at(incrementing) == incrementing->buffer);
	circular_buffer_read(incrementing, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_starts_at(incrementing) == incrementing->buffer + READ_SIZE);

	CU_ASSERT(circular_buffer_starts_at(empty) == empty->buffer);
	circular_buffer_read(empty, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_starts_at(empty) == empty->buffer);
}

void test_circular_buffer_ends_at(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;
	char testdata[] = { 19, 84 };

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(20);

	CU_ASSERT(circular_buffer_ends_at(incrementing) == incrementing->buffer + incrementing->length);
	circular_buffer_read(incrementing, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_ends_at(incrementing) == incrementing->buffer + incrementing->length);
	circular_buffer_write(incrementing, testdata, sizeof(testdata));
	/* make sure head wraps */
	CU_ASSERT(circular_buffer_ends_at(incrementing) == incrementing->buffer + sizeof(testdata) - 1);
}

void test_circular_buffer_available_data(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(20);

	CU_ASSERT(circular_buffer_available_data(incrementing) == SIZE);
	circular_buffer_read(incrementing, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_available_data(incrementing) == SIZE - READ_SIZE);

	CU_ASSERT(circular_buffer_available_data(empty) == 0);
	circular_buffer_read(empty, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_available_data(empty) == 0);
}

void test_circular_buffer_available_space(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5, WRITE_SIZE = 5;
	char buffer[SIZE], data[SIZE];
	struct circular_buffer *incrementing, *empty;
	int i;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(SIZE);

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	CU_ASSERT(circular_buffer_available_space(incrementing) == 0);
	circular_buffer_read(incrementing, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_available_space(incrementing) == READ_SIZE);

	CU_ASSERT(circular_buffer_available_space(empty) == SIZE);
	circular_buffer_read(empty, buffer, READ_SIZE);
	CU_ASSERT(circular_buffer_available_space(empty) == SIZE);
	circular_buffer_write(empty, data, WRITE_SIZE);
	CU_ASSERT(circular_buffer_available_space(empty) == SIZE - WRITE_SIZE);
}

static CU_TestInfo tests_utilities[] = {
	{ "circular_buffer_full",            test_circular_buffer_full },
	{ "circular_buffer_empty",           test_circular_buffer_empty },
	{ "circular_buffer_starts_at",       test_circular_buffer_starts_at },
	{ "circular_buffer_ends_at",         test_circular_buffer_ends_at },
	{ "circular_buffer_available_data",  test_circular_buffer_available_data },
	{ "circular_buffer_available_space", test_circular_buffer_available_space },
	CU_TEST_INFO_NULL,
};

void test_circular_buffer_read_all(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(20);

	CU_ASSERT(circular_buffer_available_data(incrementing) == incrementing->length);
	ret = circular_buffer_read(incrementing, buffer, sizeof(buffer));
	CU_ASSERT(ret == sizeof(buffer));
	CU_ASSERT(circular_buffer_available_data(incrementing) == 0);
	for (i = 0; i < sizeof(buffer); i++) {
		if (buffer[i] != i) {
			CU_FAIL("Data read from incrementing is incorrect.");
			break;
		}
	}
}

void test_circular_buffer_read_none(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE];
	struct circular_buffer *incrementing, *empty;
	int ret;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(20);

	CU_ASSERT(circular_buffer_available_data(incrementing) == incrementing->length);
	ret = circular_buffer_read(incrementing, buffer, 0);
	CU_ASSERT(ret == 0);
	CU_ASSERT(circular_buffer_available_data(incrementing) == incrementing->length);

	CU_ASSERT(circular_buffer_available_data(empty) == 0);
	ret = circular_buffer_read(empty, buffer, 0);
	CU_ASSERT(ret == 0);
	CU_ASSERT(circular_buffer_available_data(empty) == 0);
}

void test_circular_buffer_read_one_at_a_time(void)
{
}

void test_circular_buffer_read_too_much(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE + 1];
	struct circular_buffer *incrementing, *empty;
	int ret;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(20);

	/*
	 * circular_buffer_read should only return up to the amount of available data. This
	 * means it should return at most the amount specified, but could be less if the amount
	 * specified was more than was available.
	 */
	CU_ASSERT(circular_buffer_available_data(incrementing) == incrementing->length);
	ret = circular_buffer_read(incrementing, buffer, sizeof(buffer));
	CU_ASSERT(ret == SIZE);
	CU_ASSERT(circular_buffer_available_data(incrementing) == 0);

	CU_ASSERT(circular_buffer_available_data(empty) == 0);
	ret = circular_buffer_read(empty, buffer, 1);
	CU_ASSERT(ret == 0);
	CU_ASSERT(circular_buffer_available_data(empty) == 0);
}

static CU_TestInfo tests_read[] = {
	{ "circular_buffer_read_all",           test_circular_buffer_read_all },
	{ "circular_buffer_read_none",          test_circular_buffer_read_none },
	{ "circular_buffer_read_one_at_a_time", test_circular_buffer_read_one_at_a_time },
	{ "circular_buffer_read_too_much",      test_circular_buffer_read_too_much },
	CU_TEST_INFO_NULL,
};

void test_circular_buffer_write_all(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char buffer[SIZE], data[SIZE];
	struct circular_buffer *incrementing, *empty;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(SIZE);

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	CU_ASSERT(circular_buffer_available_data(empty) == 0);
	ret = circular_buffer_write(empty, data, SIZE);
	CU_ASSERT(ret == SIZE);
	CU_ASSERT(circular_buffer_available_data(empty) == SIZE);
	for (i = 0; i < empty->length; i++) {
		if (empty->buffer[i] != i) {
			CU_FAIL("Data written to empty buffer is incorrect.");
			break;
		}
	}
}

void test_circular_buffer_write_none(void)
{
}

void test_circular_buffer_write_one_at_a_time(void)
{
}

void test_circular_buffer_write_too_much(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5;
	char data[SIZE + 1];
	struct circular_buffer *incrementing, *empty;
	int i, ret;

	empty = circular_buffer_create(SIZE);

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	CU_ASSERT(circular_buffer_available_data(empty) == 0);
	ret = circular_buffer_write(empty, data, sizeof(data));
	CU_ASSERT(ret == -1);
	CU_ASSERT(circular_buffer_available_data(empty) == 0);
	for (i = 0; i < empty->length; i++) {
		if (empty->buffer[i] != 0) {
			CU_FAIL("The buffer was modified when it shouldn't have been.");
			break;
		}
	}
}

static CU_TestInfo tests_write[] = {
	{ "circular_buffer_write_all",           test_circular_buffer_write_all },
	{ "circular_buffer_write_none",          test_circular_buffer_write_none },
	{ "circular_buffer_write_one_at_a_time", test_circular_buffer_write_one_at_a_time },
	{ "circular_buffer_write_too_much",      test_circular_buffer_write_too_much },
	CU_TEST_INFO_NULL,
};

void test_circular_buffer_head_wraps(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 5, WRITE_SIZE = 5;
	char data[SIZE];
	struct circular_buffer *incrementing, *empty;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(SIZE);

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	/* Attempt to write to a full buffer and expect nothing to change. */
	CU_ASSERT(incrementing->head == incrementing->length);
	CU_ASSERT(incrementing->tail == 0);
	ret = circular_buffer_write(incrementing, data, WRITE_SIZE);
	CU_ASSERT(ret == -1);
	CU_ASSERT(incrementing->head == incrementing->length);
	CU_ASSERT(incrementing->tail == 0);
	/* Now read some. */
	ret = circular_buffer_read(incrementing, data, READ_SIZE);
	CU_ASSERT(ret == READ_SIZE);
	CU_ASSERT(incrementing->head == incrementing->length);
	CU_ASSERT(incrementing->tail == READ_SIZE);
	/* Now write and verify head has wrapped. */
	ret = circular_buffer_write(incrementing, data, WRITE_SIZE);
	CU_ASSERT(ret == WRITE_SIZE);
	CU_ASSERT(incrementing->head == WRITE_SIZE - 1);
	CU_ASSERT(incrementing->tail == READ_SIZE);
}

void test_circular_buffer_tail_wraps(void)
{
	const unsigned int SIZE = 10, READ_SIZE = 4, WRITE_SIZE = 5;
	char data[SIZE];
	struct circular_buffer *incrementing, *empty;
	int i, ret;

	incrementing = incrementing_buffer(SIZE);
	empty = circular_buffer_create(SIZE);

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	/* Attempt to write to a full buffer and expect nothing to change. */
	CU_ASSERT(incrementing->head == incrementing->length);
	CU_ASSERT(incrementing->tail == 0);
	ret = circular_buffer_write(incrementing, data, WRITE_SIZE);
	CU_ASSERT(ret == -1);
	CU_ASSERT(incrementing->head == incrementing->length);
	CU_ASSERT(incrementing->tail == 0);
	/* Now read all data. */
	ret = circular_buffer_read(incrementing, data, incrementing->length);
	CU_ASSERT(ret == incrementing->length);
	CU_ASSERT(incrementing->head == incrementing->length);
	CU_ASSERT(incrementing->tail == incrementing->length);
	/* Now fill the buffer up again. */
	ret = circular_buffer_write(incrementing, data, WRITE_SIZE);
	CU_ASSERT(ret == WRITE_SIZE);
	CU_ASSERT(incrementing->head == WRITE_SIZE - 1);
	CU_ASSERT(incrementing->tail == incrementing->length);
	/* Now read and verify tail has wrapped. */
	ret = circular_buffer_read(incrementing, data, READ_SIZE);
	CU_ASSERT(ret == READ_SIZE);
	CU_ASSERT(incrementing->head == WRITE_SIZE - 1);
	CU_ASSERT(incrementing->tail == READ_SIZE - 1);
}

static CU_TestInfo tests_read_write[] = {
	{ "circular_buffer_head_wraps", test_circular_buffer_head_wraps },
	{ "circular_buffer_tail_wraps", test_circular_buffer_tail_wraps },
	CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
	{ "suite_utilities",  NULL, NULL, tests_utilities },
	{ "suite_read",       NULL, NULL, tests_read },
	{ "suite_write",      NULL, NULL, tests_write },
	{ "suite_read_write", NULL, NULL, tests_read_write },
	CU_SUITE_INFO_NULL,
};

int main(int argc, char **argv)
{
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	if (CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "suite registration failed: %s\n", CU_get_error_msg());
		exit(EXIT_FAILURE);
	}

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_set_error_action(CUEA_IGNORE);
	CU_basic_run_tests();

fail:
	CU_cleanup_registry();
	return CU_get_error();
}
