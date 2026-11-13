#include "bme280.h"

#define REG_ID 0xD0U
#define REG_RESET 0xE0U
#define REG_CTRL_HUM 0xF2U
#define REG_CTRL_MEAS 0xF4U
#define REG_CONFIG 0xF5U
#define REG_PRESS_MSB 0xF7U

static uint16_t ReadU16(const uint8_t *data)
{
    return (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
}

static int16_t ReadS16(const uint8_t *data)
{
    return (int16_t)ReadU16(data);
}

static HAL_StatusTypeDef ReadReg(BME280_HandleTypeDef *s, uint8_t reg,
                                 uint8_t *data, uint16_t length)
{
    return HAL_I2C_Mem_Read(s->i2c, (uint16_t)(s->address << 1), reg,
                            I2C_MEMADD_SIZE_8BIT, data, length, 200U);
}

static HAL_StatusTypeDef WriteReg(BME280_HandleTypeDef *s, uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(s->i2c, (uint16_t)(s->address << 1), reg,
                             I2C_MEMADD_SIZE_8BIT, &value, 1U, 200U);
}

static HAL_StatusTypeDef ReadCalibration(BME280_HandleTypeDef *s)
{
    uint8_t a[26];
    uint8_t h[7];

    if (ReadReg(s, 0x88U, a, sizeof(a)) != HAL_OK ||
        ReadReg(s, 0xE1U, h, sizeof(h)) != HAL_OK)
        return HAL_ERROR;

    s->digT1 = ReadU16(&a[0]);  s->digT2 = ReadS16(&a[2]);  s->digT3 = ReadS16(&a[4]);
    s->digP1 = ReadU16(&a[6]);  s->digP2 = ReadS16(&a[8]);  s->digP3 = ReadS16(&a[10]);
    s->digP4 = ReadS16(&a[12]); s->digP5 = ReadS16(&a[14]); s->digP6 = ReadS16(&a[16]);
    s->digP7 = ReadS16(&a[18]); s->digP8 = ReadS16(&a[20]); s->digP9 = ReadS16(&a[22]);
    s->digH1 = a[25]; s->digH2 = ReadS16(&h[0]); s->digH3 = h[2];
    s->digH4 = (int16_t)(((int16_t)(int8_t)h[3] * 16) | (int16_t)(h[4] & 0x0FU));
    s->digH5 = (int16_t)(((int16_t)(int8_t)h[5] * 16) | (int16_t)(h[4] >> 4));
    s->digH6 = (int8_t)h[6];
    return HAL_OK;
}

HAL_StatusTypeDef BME280_Init(BME280_HandleTypeDef *s, I2C_HandleTypeDef *i2c)
{
    uint8_t id = 0U;
    uint8_t addresses[2] = {0x76U, 0x77U};
    uint8_t i;

    s->i2c = i2c;
    s->address = 0U;

    for (i = 0U; i < 2U; i++)
    {
        if (HAL_I2C_IsDeviceReady(i2c, (uint16_t)(addresses[i] << 1), 2U, 100U) == HAL_OK)
        {
            s->address = addresses[i];
            break;
        }
    }
    if (s->address == 0U)
        return HAL_ERROR;

    if (ReadReg(s, REG_ID, &id, 1U) != HAL_OK || id != 0x60U)
        return HAL_ERROR;
    if (WriteReg(s, REG_RESET, 0xB6U) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(10U);
    if (ReadCalibration(s) != HAL_OK)
        return HAL_ERROR;

    if (WriteReg(s, REG_CTRL_HUM, 0x01U) != HAL_OK ||
        WriteReg(s, REG_CONFIG, 0x00U) != HAL_OK ||
        WriteReg(s, REG_CTRL_MEAS, 0x27U) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}

HAL_StatusTypeDef BME280_Read(BME280_HandleTypeDef *s, BME280_Data *d)
{
    uint8_t r[8];
    uint32_t pRaw, tRaw;
    uint16_t hRaw;
    float v1, v2, tFine, t, p, h;

    if (ReadReg(s, REG_PRESS_MSB, r, sizeof(r)) != HAL_OK)
        return HAL_ERROR;

    pRaw = ((uint32_t)r[0] << 12) | ((uint32_t)r[1] << 4) | (r[2] >> 4);
    tRaw = ((uint32_t)r[3] << 12) | ((uint32_t)r[4] << 4) | (r[5] >> 4);
    hRaw = (uint16_t)(((uint16_t)r[6] << 8) | r[7]);

    v1 = (((float)tRaw / 16384.0f) - ((float)s->digT1 / 1024.0f)) * (float)s->digT2;
    v2 = (((float)tRaw / 131072.0f) - ((float)s->digT1 / 8192.0f));
    v2 = v2 * v2 * (float)s->digT3;
    tFine = v1 + v2;
    t = tFine / 5120.0f;

    v1 = (tFine / 2.0f) - 64000.0f;
    v2 = v1 * v1 * ((float)s->digP6 / 32768.0f);
    v2 += v1 * (float)s->digP5 * 2.0f;
    v2 = (v2 / 4.0f) + ((float)s->digP4 * 65536.0f);
    v1 = (((float)s->digP3 * v1 * v1 / 524288.0f) +
          ((float)s->digP2 * v1)) / 524288.0f;
    v1 = (1.0f + (v1 / 32768.0f)) * (float)s->digP1;
    if (v1 == 0.0f)
        return HAL_ERROR;

    p = 1048576.0f - (float)pRaw;
    p = (p - (v2 / 4096.0f)) * 6250.0f / v1;
    v1 = (float)s->digP9 * p * p / 2147483648.0f;
    v2 = p * (float)s->digP8 / 32768.0f;
    p += (v1 + v2 + (float)s->digP7) / 16.0f;

    h = tFine - 76800.0f;
    h = ((float)hRaw - (((float)s->digH4 * 64.0f) +
         ((float)s->digH5 / 16384.0f) * h)) *
        (((float)s->digH2 / 65536.0f) *
         (1.0f + (((float)s->digH6 / 67108864.0f) * h *
         (1.0f + ((float)s->digH3 / 67108864.0f) * h))));
    h *= (1.0f - ((float)s->digH1 * h / 524288.0f));

    if (h > 100.0f) h = 100.0f;
    if (h < 0.0f) h = 0.0f;
    if (p < 0.0f) return HAL_ERROR;

    d->temperatureCentiC = (int32_t)(t * 100.0f);
    d->pressurePa = (uint32_t)(p + 0.5f);
    d->humidityCentiPercent = (uint32_t)(h * 100.0f);
    return HAL_OK;
}
