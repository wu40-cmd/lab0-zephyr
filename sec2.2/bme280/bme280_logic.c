#include "bme280_logic.h"

static uint16_t read_le16(const uint8_t *raw)
{
	return (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
}

bool bme280_chip_id_is_valid(uint8_t chip_id)
{
	return chip_id == BME280_EXPECTED_CHIP_ID;
}

void bme280_parse_temperature_calibration(
	const uint8_t raw[6], struct bme280_temperature_calibration *calibration)
{
	calibration->dig_t1 = read_le16(&raw[0]);
	calibration->dig_t2 = (int16_t)read_le16(&raw[2]);
	calibration->dig_t3 = (int16_t)read_le16(&raw[4]);
}

int32_t bme280_decode_temperature_raw(const uint8_t raw[3])
{
	return ((int32_t)raw[0] << 12) |
	       ((int32_t)raw[1] << 4) |
	       ((int32_t)raw[2] >> 4);
}

int32_t bme280_compensate_temperature(
	int32_t adc_temperature,
	const struct bme280_temperature_calibration *calibration)
{
	int32_t var1;
	int32_t var2;
	int32_t t_fine;

	var1 = (((adc_temperature >> 3) - ((int32_t)calibration->dig_t1 << 1)) *
		(int32_t)calibration->dig_t2) >> 11;
	var2 = (((((adc_temperature >> 4) - (int32_t)calibration->dig_t1) *
		  ((adc_temperature >> 4) - (int32_t)calibration->dig_t1)) >> 12) *
		(int32_t)calibration->dig_t3) >> 14;

	t_fine = var1 + var2;
	return (t_fine * 5 + 128) >> 8;
}
