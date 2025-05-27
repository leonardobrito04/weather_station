#define BH1750_ADDR                 0x23    // Endereço I2C (pode ser 0x23 ou 0x5C)
#define BH1750_POWER_DOWN           0x00
#define BH1750_POWER_ON             0x01
#define BH1750_RESET                0x07
#define BH1750_CONTINUOUS_H_RES_MODE 0x10   // Modo de alta resolução (1 lx)
#define BH1750_ONE_TIME_H_RES_MODE  0x20    // Modo de alta resolução (1 lx) - leitura única