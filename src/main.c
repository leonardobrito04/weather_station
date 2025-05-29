#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "sdkconfig.h"
#include "bme280.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define TAG "BME280_MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "Sistema inicializado");
    ESP_LOGI(TAG, "Criando tarefa para leitura do BME280");

    // Cria a tarefa para leitura do sensor
    xTaskCreate(
        read_bme_280_task,    // Função da tarefa
        "bme280_task",        // Nome da tarefa
        4096,                 // Tamanho da stack (pode ajustar conforme necessidade)
        NULL,                 // Parâmetros para a tarefa
        5,                    // Prioridade (0-24, sendo 24 a mais alta)
        NULL                  // Handle para referência à tarefa
    );

    ESP_LOGI(TAG, "Tarefa criada com sucesso");
}