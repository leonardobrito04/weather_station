#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "math.h"
#include "bme280.h"

#define I2C_MASTER_TIMEOUT_MS 1000

static void i2c_master_init_bus(i2c_master_bus_handle_t *bus_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

static void i2c_master_init_handle(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle, uint8_t address)
{
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 100000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

static esp_err_t read_byte_i2c(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t write_byte_i2c(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

void read_bme_280_task(void *arg)
{
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    bme280_calib_data_t calib;
    i2c_master_init_bus(&bus_handle);
    i2c_master_init_handle(&bus_handle, &dev_handle, BME280_ADDR);

    uint8_t calib_data[32] = {0};

    read_byte_i2c(dev_handle, BME280_REG_CALIB_START, calib_data, 32);

    calib.dig_T1 = (calib_data[1] << 8) | calib_data[0];
    calib.dig_T2 = (calib_data[3] << 8) | calib_data[2];
    calib.dig_T3 = (calib_data[5] << 8) | calib_data[4];

    calib.dig_P1 = (calib_data[7] << 8) | calib_data[6];
    calib.dig_P2 = (calib_data[9] << 8) | calib_data[8];
    calib.dig_P3 = (calib_data[11] << 8) | calib_data[10];
    calib.dig_P4 = (calib_data[13] << 8) | calib_data[12];
    calib.dig_P5 = (calib_data[15] << 8) | calib_data[14];
    calib.dig_P6 = (calib_data[17] << 8) | calib_data[16];
    calib.dig_P7 = (calib_data[19] << 8) | calib_data[18];
    calib.dig_P8 = (calib_data[21] << 8) | calib_data[20];
    calib.dig_P9 = (calib_data[23] << 8) | calib_data[22];

    calib.dig_H1 = calib_data[25];
    calib.dig_H2 = (calib_data[27] << 8) | calib_data[26];
    calib.dig_H3 = calib_data[27];
    calib.dig_H4 = ((calib_data[29] << 4) | (calib_data[30] & 0x0F));
    calib.dig_H5 = ((calib_data[31] << 4) | (calib_data[30] >> 4));
    calib.dig_H6 = calib_data[32];

    for (;;)
    {
        write_byte_i2c(dev_handle, BME280_REG_CTRL_MEAS, 0x25);
        write_byte_i2c(dev_handle, BME280_REG_CTRL_HUM, 0x01);

        uint8_t temp_data[3], press_data[3], humidity_data[2];

        read_byte_i2c(dev_handle, BME280_REG_TEMP_MSB, temp_data, 3);
        read_byte_i2c(dev_handle, BME280_REG_PRESS_MSB, press_data, 3);
        read_byte_i2c(dev_handle, BME280_REG_HUM_MSB, humidity_data, 2);

        int32_t adc_T = (temp_data[0] << 12) | (temp_data[1] << 4) | (temp_data[2] >> 4);
        int32_t compensated_temp = compensate_temperature(adc_T, &calib);
        float temperature = compensated_temp / 100.0f;

        int32_t adc_P = (press_data[0] << 12) | (press_data[1] << 4) | (press_data[2] >> 4);
        int32_t compensated_pressure = compensate_pressure(adc_P, &calib);
        float pressure = compensated_pressure / 25600.0f;

        uint16_t adc_H = (humidity_data[0] << 8) | humidity_data[1];
        int32_t compensated_humidity = compensate_humidity(adc_H, &calib);
        float humidity = compensated_humidity / 1024.0f;

        ESP_LOGI("BME280", "Temperatura: %.2f °C, Pressao: %.2f hPa, Umidade: %.2f %%", temperature, pressure, humidity);

        vTaskDelay(2000 / portTICK_PERIOD_MS); 
    }
}
