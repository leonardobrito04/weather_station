#include "bme280.h"
//funcoes disponiveis no datasheet
int32_t t_fine; // Variável global utilizada nas compensações de pressão e umidade

int32_t compensate_temperature(int32_t adc_T, bme280_calib_data_t *calib)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)calib->dig_T1 << 1))) * ((int32_t)calib->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib->dig_T1)) * ((adc_T >> 4) - ((int32_t)calib->dig_T1))) >> 12) *
            ((int32_t)calib->dig_T3)) >>
           14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T; // Temperatura em centésimos de grau Celsius (ex: 5123 = 51.23°C)
}

uint32_t compensate_pressure(int32_t adc_P, bme280_calib_data_t *calib)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib->dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib->dig_P5) << 17);
    var2 = var2 + (((int64_t)calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib->dig_P3) >> 8) + ((var1 * (int64_t)calib->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib->dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0; // Evita divisão por zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib->dig_P7) << 4);
    return (uint32_t)p; // Pressão em Pa (ex: 100325 = 1003.25 hPa)
}

int32_t compensate_humidity(int32_t adc_H, bme280_calib_data_t *calib)
{
    int32_t v_x1_u32r;

    v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((adc_H << 14) - ((int32_t)calib->dig_H4 << 20) - 
                   ((int32_t)calib->dig_H5 * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * (int32_t)calib->dig_H6) >> 10) *
                      (((v_x1_u32r * (int32_t)calib->dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
                    (int32_t)calib->dig_H2 + 8192) >> 14));

    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                              (int32_t)calib->dig_H1) >> 4);

    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;

    return v_x1_u32r >> 12; // resultado em %RH com fator de escala de 1024
}
