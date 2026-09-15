#include <zephyr/ztest.h>

#include "bme280_logic.h"

ZTEST(bme280_test_suite, test_devicetree_contract_constants)
{
	zassert_equal(BME280_I2C_ADDRESS, 0x77,
		      "BME280 assignment address must be 0x77");
	zassert_true(bme280_chip_id_is_valid(0x60),
		     "BME280 chip ID 0x60 should be accepted");
	zassert_false(bme280_chip_id_is_valid(0x58),
		      "BMP280 chip ID must not be accepted as BME280");
}

ZTEST(bme280_test_suite, test_temperature_calibration_byte_order)
{
	const uint8_t raw[6] = {0x70, 0x6B, 0x43, 0x67, 0x18, 0xFC};
	struct bme280_temperature_calibration calibration;

	bme280_parse_temperature_calibration(raw, &calibration);

	zassert_equal(calibration.dig_t1, 27504, "incorrect dig_T1");
	zassert_equal(calibration.dig_t2, 26435, "incorrect dig_T2");
	zassert_equal(calibration.dig_t3, -1000, "incorrect dig_T3");
}

ZTEST(bme280_test_suite, test_temperature_raw_decode)
{
	const uint8_t raw[3] = {0x7E, 0xED, 0x00};

	zassert_equal(bme280_decode_temperature_raw(raw), 519888,
		      "20-bit temperature ADC value decoded incorrectly");
}

ZTEST(bme280_test_suite, test_temperature_compensation_datasheet_example)
{
	const struct bme280_temperature_calibration calibration = {
		.dig_t1 = 27504,
		.dig_t2 = 26435,
		.dig_t3 = -1000,
	};

	zassert_within(bme280_compensate_temperature(519888, &calibration),
		       2508, 1, "expected approximately 25.08 C");
}

ZTEST_SUITE(bme280_test_suite, NULL, NULL, NULL, NULL, NULL);
