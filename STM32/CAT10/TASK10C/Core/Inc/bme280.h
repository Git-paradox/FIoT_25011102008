#ifndef BME280_H
#define BME280_H

#include "stm32f4xx_hal.h"

typedef struct
{
    int32_t temperatureCentiC;
    uint32_t pressurePa;
    uint32_t humidityCentiPercent;
} BME280_Data;

typedef struct
{
    I2C_HandleTypeDef *i2c;
    uint8_t address;
    uint16_t digT1;
    int16_t digT2;
    int16_t digT3;
    uint16_t digP1;
    int16_t digP2;
    int16_t digP3;
    int16_t digP4;
    int16_t digP5;
    int16_t digP6;
    int16_t digP7;
    int16_t digP8;
    int16_t digP9;
    uint8_t digH1;
    int16_t digH2;
    uint8_t digH3;
    int16_t digH4;
    int16_t digH5;
    int8_t digH6;
} BME280_HandleTypeDef;

HAL_StatusTypeDef BME280_Init(BME280_HandleTypeDef *sensor, I2C_HandleTypeDef *i2c);
HAL_StatusTypeDef BME280_Read(BME280_HandleTypeDef *sensor, BME280_Data *data);

#endif
