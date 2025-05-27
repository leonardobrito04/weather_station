#include "sensors.h"

void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main() {
    // Inicializa I2C
    i2c_master_init();

    // Cria tasks
    /*xTaskCreate(
        bh1750_task,    
        "BH1750_Task",  
        2048,           
        NULL,           
        5,              
        NULL           
    );*/

    xTaskCreate(
        bme280_task,        // Função da task
        "bme280_task",      // Nome da task
        4096,               // Tamanho da stack (aumente se necessário)
        NULL,               // Parâmetros
        5,                  // Prioridade (0-25)
        NULL                // Handle da task
    );

    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}