/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "bme280.h"

#ifdef CONFIG_SUM_PRINT
#include "sum_printk.h"
#elif defined(CONFIG_SUM_LOG)
#include "sum_log.h"
#endif

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   2000
#define BUTTON_POLL_MS  50

/* The devicetree node identifier for the "led0" alias. */
#define LED5180_NODE DT_ALIAS(led5180)
#define BUTTON5180_NODE DT_ALIAS(button5180)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED5180_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON5180_NODE, gpios);

int main(void)
{
	int ret;
	bool button_was_pressed = false;
	int64_t next_sample_time;
	int32_t temperature_centi_c;

	int result = sum(10, 20);
	(void)result;

	if (!gpio_is_ready_dt(&led)) {
		printk("LED GPIO is not ready\n");
		return 0;
	}

	if (!gpio_is_ready_dt(&button)) {
		printk("Button GPIO is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("Failed to configure LED: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure button: %d\n", ret);
		return 0;
	}

	ret = bme280_init();
	if (ret < 0) {
		printk("Failed to initialize BME280: %d\n", ret);
		return 0;
	}

	next_sample_time = k_uptime_get();

	while (1) {
		int button_pressed = gpio_pin_get_dt(&button);

		if (button_pressed < 0) {
			printk("Failed to read button: %d\n", button_pressed);
			return 0;
		}

		if (button_pressed && !button_was_pressed) {
			ret = gpio_pin_toggle_dt(&led);
			if (ret < 0) {
				printk("Failed to toggle LED: %d\n", ret);
				return 0;
			}
			printk("Button pressed: LED toggled\n");
		}
		button_was_pressed = button_pressed;

		if (k_uptime_get() >= next_sample_time) {
			ret = bme280_read_temperature(&temperature_centi_c);
			if (ret < 0) {
				printk("Failed to read BME280 temperature: %d\n", ret);
			} else {
				int32_t magnitude = temperature_centi_c < 0
					? -temperature_centi_c : temperature_centi_c;

				printk("Temperature: %s%d.%02d C\n",
				       temperature_centi_c < 0 ? "-" : "",
				       magnitude / 100, magnitude % 100);
			}
			next_sample_time += SLEEP_TIME_MS;
		}

		k_msleep(BUTTON_POLL_MS);
	}
	return 0;
}
