#include "sensors.h"
#include "esp_log.h"
#include "BH1750.h"

static const char *TAG = "BH1750";

void bh1750_task(void *pvParameters)
{
    uint8_t cmd = BH1750_POWER_ON;
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, BH1750_ADDR, &cmd, 1, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao ligar o sensor: %d", ret);
    }

    cmd = BH1750_CONTINUOUS_H_RES_MODE;
    i2c_master_write_to_device(I2C_MASTER_NUM, BH1750_ADDR, &cmd, 1, I2C_MASTER_TIMEOUT);
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, BH1750_ADDR, &cmd, 1, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Falha ao configurar modo: %d", ret);
    }

    for (;;)
    {
        uint8_t data[2] = {0};
        esp_err_t ret = i2c_master_read_from_device(I2C_MASTER_NUM, BH1750_ADDR, data, 2, pdMS_TO_TICKS(1000));
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Falha na leitura: %d", ret);
        }

        // Converte os 2 bytes em um valor de 16 bits
        uint16_t raw = (data[0] << 8) | data[1];
        float lux = raw / 2.4;
        ESP_LOGI(TAG, "Luminosidade: %.2f lux", lux);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}