// SPDX-License-Identifier: GPL-2.0
#include <string.h>
#include "tests/tests.h"
#include "arch-tests.h"

#include <unistd.h>
#include <sched.h>
#include <cpumap.h>

#include <perf/core.h>
#include <perf/threadmap.h>
#include <perf/evsel.h>

#include "pmu.h"
#include "debug.h"
#include "tests/tests.h"
#include "arch-tests.h"

static int test__rd_counter_size(struct test_suite *test,
				 int config1)
{
	int ret;
	struct perf_evsel *evsel;
	struct perf_event_attr attr = {
		.type = PERF_TYPE_HARDWARE,
		.config = PERF_COUNT_HW_INSTRUCTIONS,
		.config1 = config1,
		.exclude_kernel = 1,
	};

	if (!pmu_is_homogeneous())
		return TEST_SKIP;

	evsel = perf_init(&attr);
	if (!evsel)
		return -1;

	ret = run_test(evsel);

	perf_evsel__close(evsel);
	perf_evsel__delete(evsel);
	return ret;
}

int test__rd_64bit(struct test_suite *test,
		   int subtest)
{
	return test__rd_counter_size(test, 0x3);
}

int test__rd_32bit(struct test_suite *test,
		   int subtest)
{
	return test__rd_counter_size(test, 0x2);
}

struct test_suite test__rd_64bit = {
		.desc = "User 64-bit counter access",
		.func = _test__rd_64bit,
};

struct test_suite test__rd_32bit = {
		.desc = "User 32-bit counter access",
		.func = _test__rd_32bit,
};

struct test_suite *arch_tests[] = {
#ifdef HAVE_DWARF_UNWIND_SUPPORT
	&suite__dwarf_unwind,
#endif
	&test__rd_64bit,
	&test__rd_32bit,
	NULL,
};
