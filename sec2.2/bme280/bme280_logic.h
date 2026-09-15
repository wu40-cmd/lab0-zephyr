#ifndef BME280_LOGIC_H
#define BME280_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#define BME280_I2C_ADDRESS 0x77
#define BME280_EXPECTED_CHIP_ID 0x60

struct bme280_temperature_calibration {
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
};

bool bme280_chip_id_is_valid(uint8_t chip_id);
void bme280_parse_temperature_calibration(
	const uint8_t raw[6], struct bme280_temperature_calibration *calibration);
int32_t bme280_decode_temperature_raw(const uint8_t raw[3]);
int32_t bme280_compensate_temperature(
	int32_t adc_temperature,
	const struct bme280_temperature_calibration *calibration);

#endif
