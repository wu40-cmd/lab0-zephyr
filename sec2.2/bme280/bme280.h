#ifndef LAB0_BME280_H
#define LAB0_BME280_H

#include <stdint.h>

int bme280_init(void);
int bme280_read_temperature(int32_t *temperature_centi_c);

#endif
