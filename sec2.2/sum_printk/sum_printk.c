#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "sum_printk.h"

int sum(int a, int b)
{
    int result = a + b;

    printk("printk: %d + %d = %d\n", a, b, result);

    return result;
}
