#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sum_log.h"

LOG_MODULE_REGISTER(sum_log);

int sum(int a, int b)
{
    int result = a + b;
    int inputs[2] = {a, b};

    LOG_INF("Sum: %d + %d = %d", a, b, result);
    LOG_WRN("This is a warning log");
    LOG_ERR("This is an error log");

    LOG_HEXDUMP_INF(inputs, sizeof(inputs), "Input values:");

    return result;
}
