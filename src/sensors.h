#ifndef SENSORS_H
#define SENSORS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

// Definições comuns
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_TIMEOUT  1000 / portTICK_PERIOD_MS

// Inicialização I2C
void i2c_master_init(void);

// BH1750
void bh1750_task(void *pvParameters);

// BME280
void bme280_task(void *pvParameters);

#endif