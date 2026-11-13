#include "main.h"
#include "bme280.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

static struct bme280_dev dev;
static struct bme280_data comp_data;
static int8_t rslt;
static uint8_t dev_addr = BME280_I2C_ADDR_PRIM;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void UART_Print(const char *text);
static void ReadAndPrintSensor(void);

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                     uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(address << 1),
                         reg_addr, I2C_MEMADD_SIZE_8BIT,
                         reg_data, (uint16_t)len, HAL_MAX_DELAY) == HAL_OK)
        return BME280_OK;

    return BME280_E_COMM_FAIL;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data,
                      uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(address << 1),
                          reg_addr, I2C_MEMADD_SIZE_8BIT,
                          (uint8_t *)reg_data, (uint16_t)len,
                          HAL_MAX_DELAY) == HAL_OK)
        return BME280_OK;

    return BME280_E_COMM_FAIL;
}

void user_delay_us(uint32_t period, void *intf_ptr)
{
    HAL_Delay((period / 1000U) + 1U);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();

    UART_Print("\r\nCAT12A - BME280 Bosch SensorAPI\r\n");

    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    rslt = bme280_init(&dev);
    if (rslt != BME280_OK)
    {
        UART_Print("BME280 initialization failed. Check I2C wiring and address.\r\n");
        Error_Handler();
    }

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_1X;
    dev.settings.osr_t = BME280_OVERSAMPLING_1X;
    dev.settings.filter = BME280_FILTER_COEFF_OFF;
    dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS;

    rslt = bme280_set_sensor_settings(
        BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP |
        BME280_SEL_OSR_HUM | BME280_SEL_FILTER |
        BME280_SEL_STANDBY, &dev);

    if (rslt != BME280_OK)
    {
        UART_Print("BME280 sensor configuration failed.\r\n");
        Error_Handler();
    }

    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);
    if (rslt != BME280_OK)
    {
        UART_Print("BME280 normal mode could not be selected.\r\n");
        Error_Handler();
    }

    UART_Print("BME280 ready. Reading every second.\r\n");

    while (1)
    {
        ReadAndPrintSensor();
        HAL_Delay(1000U);
    }
}

static void ReadAndPrintSensor(void)
{
    char line[128];
    double temperature;
    double pressure;
    double humidity;

    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);

    if (rslt != BME280_OK)
    {
        UART_Print("Sensor read failed.\r\n");
        return;
    }

    temperature = comp_data.temperature;
    pressure = comp_data.pressure / 100.0;
    humidity = comp_data.humidity;

    (void)snprintf(line, sizeof(line),
                   "Temperature: %.2f C | Pressure: %.2f hPa | Humidity: %.2f %%RH\r\n",
                   temperature, pressure, humidity);
    UART_Print(line);
}

static void UART_Print(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text,
                      (uint16_t)strlen(text), HAL_MAX_DELAY);
}

void SystemClock_Config(void)
{
    /* Use the internal 16 MHz HSI clock. */
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600U;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
        Error_Handler();
}

static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000U;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0U;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0U;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
        Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
