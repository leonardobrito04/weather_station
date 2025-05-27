#ifndef BME280_H
#define BME280_H

#include "driver/i2c.h"
#include "esp_log.h"
#include <stdint.h>

// Endereço I2C
#define BME280_ADDR 0x77

// Registros
#define BME280_REG_ID 0xD0
#define BME280_REG_RESET 0xE0
#define BME280_REG_STATUS 0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG 0xF5
#define BME280_REG_PRESS_MSB 0xF7
#define BME280_REG_TEMP_MSB 0xFA
#define BME280_REG_HUM_MSB 0xFD 

// Modos de operação
#define BME280_SLEEP_MODE 0x00
#define BME280_FORCED_MODE 0x01
#define BME280_NORMAL_MODE 0x03

// Parâmetros de calibração
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    int16_t dig_H1, dig_H2, dig_H3, dig_H4, dig_H5, dig_H6;
} bme280_calib_t;

// Estrutura do sensor
typedef struct {
    i2c_port_t i2c_port;
    bme280_calib_t calib;
    float temperature;
    float pressure;
    float humidity;
} bme280_t;

// Protótipos de função
esp_err_t bme280_init(bme280_t *dev, i2c_port_t port);
esp_err_t bme280_read_data(bme280_t *dev);
void bme280_reset(bme280_t *dev);

#endif