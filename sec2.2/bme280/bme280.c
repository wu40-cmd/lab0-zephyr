#include "bme280.h"

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include "bme280_logic.h"

#define BME280_NODE DT_NODELABEL(bme280_5180)

#define BME280_REG_CALIB_T1 0x88
#define BME280_REG_CHIP_ID 0xD0
#define BME280_REG_STATUS 0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_TEMP_MSB 0xFA

#define BME280_STATUS_MEASURING BIT(3)
#define BME280_STATUS_IM_UPDATE BIT(0)
#define BME280_CTRL_MEAS_TEMP_X1_FORCED 0x21
#define BME280_READY_TIMEOUT_MS 20

BUILD_ASSERT(DT_NODE_EXISTS(BME280_NODE), "BME280 devicetree node is missing");
BUILD_ASSERT(DT_NODE_HAS_STATUS(BME280_NODE, okay),
	     "BME280 devicetree node must be enabled");
BUILD_ASSERT(DT_REG_ADDR(BME280_NODE) == BME280_I2C_ADDRESS,
	     "BME280 devicetree address must be 0x77");

static const struct i2c_dt_spec bme280 = I2C_DT_SPEC_GET(BME280_NODE);
static struct bme280_temperature_calibration temperature_calibration;
static bool initialized;

static int read_registers(uint8_t start_register, uint8_t *data, size_t length)
{
	return i2c_burst_read_dt(&bme280, start_register, data, length);
}

static int write_register(uint8_t reg, uint8_t value)
{
	uint8_t command[2] = {reg, value};

	return i2c_write_dt(&bme280, command, sizeof(command));
}

static int wait_until_ready(void)
{
	int64_t deadline = k_uptime_get() + BME280_READY_TIMEOUT_MS;
	uint8_t status;
	int ret;

	do {
		ret = read_registers(BME280_REG_STATUS, &status, sizeof(status));
		if (ret < 0) {
			return ret;
		}
		if ((status & (BME280_STATUS_MEASURING |
			       BME280_STATUS_IM_UPDATE)) == 0) {
			return 0;
		}
		k_msleep(1);
	} while (k_uptime_get() < deadline);

	return -ETIMEDOUT;
}

int bme280_init(void)
{
	uint8_t calibration_raw[6];
	uint8_t chip_id;
	int ret;

	if (!i2c_is_ready_dt(&bme280)) {
		return -ENODEV;
	}

	ret = read_registers(BME280_REG_CHIP_ID, &chip_id, sizeof(chip_id));
	if (ret < 0) {
		return ret;
	}
	if (!bme280_chip_id_is_valid(chip_id)) {
		return -ENODEV;
	}

	ret = wait_until_ready();
	if (ret < 0) {
		return ret;
	}

	ret = read_registers(BME280_REG_CALIB_T1, calibration_raw,
			     sizeof(calibration_raw));
	if (ret < 0) {
		return ret;
	}

	bme280_parse_temperature_calibration(calibration_raw,
					     &temperature_calibration);
	initialized = true;
	return 0;
}

int bme280_read_temperature(int32_t *temperature_centi_c)
{
	uint8_t raw_temperature[3];
	int32_t adc_temperature;
	int ret;

	if (!initialized || temperature_centi_c == NULL) {
		return -EINVAL;
	}

	ret = write_register(BME280_REG_CTRL_MEAS,
			     BME280_CTRL_MEAS_TEMP_X1_FORCED);
	if (ret < 0) {
		return ret;
	}

	ret = wait_until_ready();
	if (ret < 0) {
		return ret;
	}

	ret = read_registers(BME280_REG_TEMP_MSB, raw_temperature,
			     sizeof(raw_temperature));
	if (ret < 0) {
		return ret;
	}

	adc_temperature = bme280_decode_temperature_raw(raw_temperature);
	if (adc_temperature == 0x80000) {
		return -EIO;
	}

	*temperature_centi_c = bme280_compensate_temperature(
		adc_temperature, &temperature_calibration);
	return 0;
}
