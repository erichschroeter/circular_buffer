#include <stdlib.h>

#include <CUnit/Basic.h>

#include "circular_buffer.h"

#define TEST_DATA_SIZE 100
#define INCREMENTING_SIZE 10
#define TEST1_SIZE 10
static struct circular_buffer *incrementing;
static struct circular_buffer *empty;
static struct circular_buffer *test1;
static struct circular_buffer *test2;

void test_circular_buffer_full(void)
{
	CU_ASSERT_TRUE(circular_buffer_full(incrementing));
	CU_ASSERT_FALSE(circular_buffer_full(empty));
	CU_ASSERT_FALSE(circular_buffer_full(test1));
}

void test_circular_buffer_empty(void)
{
	CU_ASSERT_FALSE(circular_buffer_empty(incrementing));
	CU_ASSERT_TRUE(circular_buffer_empty(empty));
	CU_ASSERT_TRUE(circular_buffer_empty(test1));
}

void test_circular_buffer_starts_at(void)
{
	char buffer[INCREMENTING_SIZE];
	const int read_size = 5;

	if (INCREMENTING_SIZE < read_size)
		CU_FAIL("Attempting to read more than buffer has.");

	CU_ASSERT(circular_buffer_starts_at(incrementing) == incrementing->buffer);
	circular_buffer_read(incrementing, buffer, read_size);
	CU_ASSERT(circular_buffer_starts_at(incrementing) == incrementing->buffer + read_size);
}

void test_circular_buffer_ends_at(void)
{
	char buffer[INCREMENTING_SIZE];
	char testdata[] = { 13, 77 };
	const int read_size = 5;

	if (INCREMENTING_SIZE < read_size)
		CU_FAIL("Attempting to read more than buffer has.");

	CU_ASSERT(circular_buffer_ends_at(incrementing) == incrementing->buffer + incrementing->length);
	circular_buffer_read(incrementing, buffer, read_size);
	CU_ASSERT(circular_buffer_ends_at(incrementing) == incrementing->buffer + incrementing->length);
	circular_buffer_write(incrementing, testdata, sizeof(testdata));
	/* make sure head wraps */
	CU_ASSERT(circular_buffer_ends_at(incrementing) == incrementing->buffer + sizeof(testdata) - 1);
}

void test_circular_buffer_available_data(void)
{
}

void test_circular_buffer_available_space(void)
{
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
	char buffer[INCREMENTING_SIZE];
	int i, ret;

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
	char buffer[INCREMENTING_SIZE];
	int ret;

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
	char buffer[INCREMENTING_SIZE + 1];
	int ret;

	/*
	 * circular_buffer_read should only return up to the amount of available data. This
	 * means it should return at most the amount specified, but could be less if the amount
	 * specified was more than was available.
	 */
	printf("\n");
	circular_buffer_debug(incrementing);
	CU_ASSERT(circular_buffer_available_data(incrementing) == incrementing->length);
	ret = circular_buffer_read(incrementing, buffer, sizeof(buffer));
	printf("returned=%d\n", ret);
	circular_buffer_debug(incrementing);
	CU_ASSERT(ret == INCREMENTING_SIZE);
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
	char data[TEST_DATA_SIZE];
	int i, ret;

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	CU_ASSERT(circular_buffer_available_data(test1) == 0);
	ret = circular_buffer_write(test1, data, TEST1_SIZE);
	CU_ASSERT(ret == TEST1_SIZE);
	CU_ASSERT(circular_buffer_available_data(test1) == TEST1_SIZE);
	for (i = 0; i < test1->length; i++) {
		if (test1->buffer[i] != i) {
			CU_FAIL("Data written to test1 is incorrect.");
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
	char data[TEST_DATA_SIZE + 1];
	int i, ret;

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	CU_ASSERT(circular_buffer_available_data(test2) == 0);
	ret = circular_buffer_write(test2, data, sizeof(data));
	CU_ASSERT(ret == -1);
	CU_ASSERT(circular_buffer_available_data(test2) == 0);
	for (i = 0; i < test2->length; i++) {
		if (test2->buffer[i] != 0) {
			CU_FAIL("test2 was modified when it shouldn't have been.");
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

int suite_test_buffers_init(void)
{
	int i;
	char data[TEST_DATA_SIZE];

	/* populate test data with incrementing numbers */
	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	incrementing = circular_buffer_create(INCREMENTING_SIZE);
	if (!incrementing)
		return -1;
	empty = circular_buffer_create(sizeof(data));
	if (!empty)
		return -1;
	test1 = circular_buffer_create(TEST1_SIZE);
	if (!test1)
		return -1;
	test2 = circular_buffer_create(TEST1_SIZE);
	if (!test2)
		return -1;

	circular_buffer_write(incrementing, data, INCREMENTING_SIZE);

#if 0
	printf("\n\n");
	printf("incrementing:\t");
	circular_buffer_debug(incrementing);
	printf("empty:\t");
	circular_buffer_debug(empty);
	printf("test1:\t");
	circular_buffer_debug(test1);
#endif

	return 0;
}

int suite_test_buffers_clean(void)
{
	circular_buffer_destroy(incrementing);
	circular_buffer_destroy(empty);
	circular_buffer_destroy(test1);

	return 0;
}

static CU_SuiteInfo suites[] = {
	{ "suite_utilities", suite_test_buffers_init, suite_test_buffers_clean, tests_utilities },
	{ "suite_read",      suite_test_buffers_init, suite_test_buffers_clean, tests_read },
	{ "suite_write",     suite_test_buffers_init, suite_test_buffers_clean, tests_write },
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
