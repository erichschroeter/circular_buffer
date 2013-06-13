#include <stdlib.h>

#include <CUnit/Basic.h>

#include "circular_buffer.h"

static struct circular_buffer *full;
static struct circular_buffer *empty;
static struct circular_buffer *test1;

void test_circular_buffer_full(void)
{
	CU_ASSERT_TRUE(circular_buffer_full(full));
	CU_ASSERT_FALSE(circular_buffer_full(empty));
	CU_ASSERT_FALSE(circular_buffer_full(test1));
}

void test_circular_buffer_empty(void)
{
	CU_ASSERT_FALSE(circular_buffer_empty(full));
	CU_ASSERT_TRUE(circular_buffer_empty(empty));
	CU_ASSERT_TRUE(circular_buffer_empty(test1));
}

void test_circular_buffer_starts_at(void)
{
}

void test_circular_buffer_ends_at(void)
{
}

void test_circular_buffer_commit_read(void)
{
}

void test_circular_buffer_commit_write(void)
{
}

static CU_TestInfo tests_utilities[] = {
	{ "test_circular_buffer_full",         test_circular_buffer_full },
	{ "test_circular_buffer_empty",        test_circular_buffer_empty },
	{ "test_circular_buffer_starts_at",    test_circular_buffer_starts_at },
	{ "test_circular_buffer_ends_at",      test_circular_buffer_ends_at },
	{ "test_circular_buffer_commit_read",  test_circular_buffer_commit_read },
	{ "test_circular_buffer_commit_write", test_circular_buffer_commit_write },
	CU_TEST_INFO_NULL,
};

int suite_test_buffers_init(void)
{
	int i;
	char data[10];

	full = circular_buffer_create(sizeof(data));
	if (!full)
		return -1;
	empty = circular_buffer_create(sizeof(data));
	if (!empty)
		return -1;
	test1 = circular_buffer_create(sizeof(data));
	if (!test1)
		return -1;

	for (i = 0; i < sizeof(data); i++)
		data[i] = i;

	circular_buffer_write(full, data, sizeof(data));

	printf("full:\t");
	circular_buffer_debug(full);
	printf("empty:\t");
	circular_buffer_debug(empty);
	printf("test1:\t");
	circular_buffer_debug(test1);

	return 0;
}

int suite_test_buffers_clean(void)
{
	circular_buffer_destroy(full);
	circular_buffer_destroy(empty);
	circular_buffer_destroy(test1);

	return 0;
}

static CU_SuiteInfo suites[] = {
	{ "suite_utilites", suite_test_buffers_init, suite_test_buffers_clean, tests_utilities },
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
