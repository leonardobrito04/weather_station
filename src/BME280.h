#ifndef BME280_H
#define BME280_H

#include <stdint.h>  

// Endereço do dispositivo (padrão com SDO em GND)
#define BME280_ADDR 0x76

// Registradores de calibração (temperatura e pressão)
#define BME280_REG_CALIB_START   0x88   // 0x88 até 0xA1 (26 bytes)

// Registradores de calibração de umidade
#define BME280_REG_CALIB_H1      0xA1
#define BME280_REG_CALIB_H_START 0xE1   // 0xE1 até 0xE7

// Registrador de controle da medição
#define BME280_REG_CTRL_HUM      0xF2   // define oversampling da umidade
#define BME280_REG_CTRL_MEAS     0xF4   // define modo de operação e oversampling de temp e pressão
#define BME280_REG_CONFIG        0xF5   // define filtros e tempo de espera

// Registradores dos dados brutos
#define BME280_REG_PRESS_MSB     0xF7   // pressão MSB
#define BME280_REG_PRESS_LSB     0xF8
#define BME280_REG_PRESS_XLSB    0xF9

#define BME280_REG_TEMP_MSB      0xFA   // temperatura MSB
#define BME280_REG_TEMP_LSB      0xFB
#define BME280_REG_TEMP_XLSB     0xFC

#define BME280_REG_HUM_MSB       0xFD   // umidade MSB
#define BME280_REG_HUM_LSB       0xFE

typedef struct {
    // Temperatura
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    // Pressão
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    // Umidade
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_data_t;

void read_bme_280_task(void *arg);
int32_t compensate_temperature(int32_t adc_T, bme280_calib_data_t *calib);
uint32_t compensate_pressure(int32_t adc_P, bme280_calib_data_t *calib);
int32_t compensate_humidity(int32_t adc_H, bme280_calib_data_t *calib);

#endif