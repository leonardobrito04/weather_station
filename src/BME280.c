#include "BME280.h"

static const char *TAG = "BME280";

void bme280_task(void *pvParameters)
{
    bme280_t sensor; // Usando a mesma struct do BME280

    // Inicializa o sensor
    if (bme280_init(&sensor, I2C_NUM_0) != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha na inicialização do BME280");
        vTaskDelete(NULL);
    }

    for (;;)
    {
        // Lê os dados do sensor
        if (bme280_read_data(&sensor) == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperatura: %.2f °C, Pressão: %.2f hPa, Umidade: %.2f",
                     sensor.temperature, sensor.pressure, sensor.humidity);
        }

        // Intervalo de leitura (2 segundos)
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Função auxiliar para leitura de registros
static esp_err_t bme280_read_register(bme280_t *dev, uint8_t reg, uint8_t *data, uint8_t len)
{
    return i2c_master_write_read_device(dev->i2c_port, BME280_ADDR, &reg, 1, data, len, pdMS_TO_TICKS(1000));
}

// Função auxiliar para escrita de registros
static esp_err_t bme280_write_register(bme280_t *dev, uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    return i2c_master_write_to_device(dev->i2c_port, BME280_ADDR, buf, sizeof(buf), pdMS_TO_TICKS(1000));
}

// Inicialização do sensor
esp_err_t bme280_init(bme280_t *dev, i2c_port_t port)
{
    dev->i2c_port = port;

    // Verifica ID do chip
    uint8_t chip_id;
    ESP_ERROR_CHECK(bme280_read_register(dev, BME280_REG_ID, &chip_id, 1));

    /*if (chip_id != 0x58) {
        ESP_LOGE(TAG, "Chip ID inválido: 0x%02X", chip_id);
        return ESP_ERR_NOT_FOUND;
    }*/

    // Reseta o sensor
    bme280_reset(dev);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Carrega parâmetros de calibração
    uint8_t calib_data[24];
    ESP_ERROR_CHECK(bme280_read_register(dev, 0x88, calib_data, 24));

    // Converte dados de calibração
    dev->calib.dig_T1 = (calib_data[1] << 8) | calib_data[0];
    dev->calib.dig_T2 = (calib_data[3] << 8) | calib_data[2];
    dev->calib.dig_T3 = (calib_data[5] << 8) | calib_data[4];

    dev->calib.dig_P1 = (calib_data[7] << 8) | calib_data[6];
    dev->calib.dig_P2 = (calib_data[9] << 8) | calib_data[8];
    dev->calib.dig_P3 = (calib_data[11] << 8) | calib_data[10];
    dev->calib.dig_P4 = (calib_data[13] << 8) | calib_data[12];
    dev->calib.dig_P5 = (calib_data[15] << 8) | calib_data[14];
    dev->calib.dig_P6 = (calib_data[17] << 8) | calib_data[16];
    dev->calib.dig_P7 = (calib_data[19] << 8) | calib_data[18];
    dev->calib.dig_P8 = (calib_data[21] << 8) | calib_data[20];
    dev->calib.dig_P9 = (calib_data[23] << 8) | calib_data[22];

    // Configura o sensor
    ESP_ERROR_CHECK(bme280_write_register(dev, BME280_REG_CONFIG, 0x00));    // Filtro desligado
    ESP_ERROR_CHECK(bme280_write_register(dev, BME280_REG_CTRL_MEAS, 0x3F)); // Oversampling máximo, modo normal

    return ESP_OK;
}

esp_err_t bme280_read_raw_data(bme280_t *dev, int32_t *adc_t, int32_t *adc_p)
{
    uint8_t data[6];
    ESP_ERROR_CHECK(bme280_read_register(dev, BME280_REG_PRESS_MSB, data, 6));

    *adc_p = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    *adc_t = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);

    return ESP_OK;
}

esp_err_t bme280_read_raw_humidity(bme280_t *dev, int32_t *adc_h)
{
    uint8_t data[2];
    esp_err_t ret = bme280_read_register(dev, BME280_REG_HUM_MSB, data, 2);
    if (ret != ESP_OK)
        return ret;

    *adc_h = (data[0] << 8) | data[1];
    return ESP_OK;
}

int32_t bme280_compensate_temperature(bme280_t *dev, int32_t adc_t)
{
    int32_t var1 = ((((adc_t >> 3) - ((int32_t)dev->calib.dig_T1 << 1))) * ((int32_t)dev->calib.dig_T2)) >> 11;
    int32_t var2 = (((((adc_t >> 4) - ((int32_t)dev->calib.dig_T1)) * ((adc_t >> 4) - ((int32_t)dev->calib.dig_T1))) >> 12) * ((int32_t)dev->calib.dig_T3)) >> 14;
    int32_t t_fine = var1 + var2;

    dev->temperature = (t_fine * 5 + 128) >> 8;
    dev->temperature /= 100.0f;

    return t_fine;
}

esp_err_t bme280_compensate_pressure(bme280_t *dev, int32_t adc_p, int32_t t_fine)
{
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)dev->calib.dig_P6;
    var2 += ((var1 * (int64_t)dev->calib.dig_P5) << 17);
    var2 += (((int64_t)dev->calib.dig_P4) << 35);
    int64_t var3 = ((var1 * var1 * (int64_t)dev->calib.dig_P3) >> 8) + ((var1 * (int64_t)dev->calib.dig_P2) << 12);
    var3 = (((((int64_t)1) << 47) + var3)) * ((int64_t)dev->calib.dig_P1) >> 33;

    if (var3 == 0)
    {
        return ESP_ERR_INVALID_RESPONSE;
    }

    int64_t p = 1048576 - adc_p;
    p = (((p << 31) - var2) * 3125) / var3;
    var2 = (((int64_t)dev->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var3 = (((int64_t)dev->calib.dig_P8) * p) >> 19;
    p = ((p + var2 + var3) >> 8) + (((int64_t)dev->calib.dig_P7) << 4);

    dev->pressure = (float)p / 25600.0f;
    return ESP_OK;
}

void bme280_compensate_humidity(bme280_t *dev, int32_t adc_h, int32_t t_fine)
{
    int32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_h << 14) - (((int32_t)dev->calib.dig_H4) << 20) - (((int32_t)dev->calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                 (((((((v_x1_u32r * ((int32_t)dev->calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dev->calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                    ((int32_t)2097152)) *
                       ((int32_t)dev->calib.dig_H2) +
                   8192) >>
                  14));

    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dev->calib.dig_H1)) >> 4);
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    dev->humidity = ((float)(v_x1_u32r >> 12)) / 1024.0f;
}

esp_err_t bme280_read_data(bme280_t *dev)
{
    int32_t adc_t, adc_p, adc_h;
    esp_err_t ret = bme280_read_raw_data(dev, &adc_t, &adc_p);
    if (ret != ESP_OK)
        return ret;

    ret = bme280_read_raw_humidity(dev, &adc_h);
    if (ret != ESP_OK)
        return ret;

    int32_t t_fine = bme280_compensate_temperature(dev, adc_t);

    ret = bme280_compensate_pressure(dev, adc_p, t_fine);
    if (ret != ESP_OK)
        return ret;

    bme280_compensate_humidity(dev, adc_h, t_fine);

    return ESP_OK;
}

// Reset do sensor
void bme280_reset(bme280_t *dev)
{
    bme280_write_register(dev, BME280_REG_RESET, 0xB6);
}