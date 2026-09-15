#include <zephyr/ztest.h>
#include "sum_log.h"

ZTEST(sum_log_test_suite, test_sum_log_basic)
{
    zassert_equal(sum(10, 20), 30, "10 + 20 should equal 30");
}

ZTEST(sum_log_test_suite, test_sum_log_negative)
{
    zassert_equal(sum(-10, -20), -30, "-10 + -20 should equal -30");
}

ZTEST(sum_log_test_suite, test_sum_log_zero)
{
    zassert_equal(sum(0, 0), 0, "0 + 0 should equal 0");
}

ZTEST_SUITE(sum_log_test_suite, NULL, NULL, NULL, NULL, NULL);
