#include "main.h"
#include "app_config.h"
#include "bme280.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static struct bme280_dev dev;
static struct bme280_data comp_data;
static uint8_t dev_addr = BME280_I2C_ADDR_PRIM;
static char requestBuffer[768];
static uint16_t requestLength = 0U;
static int clientId = 0;
static uint8_t serverReady = 0U;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void UART_Print(const char *text);
static void ESP_SendCommand(const char *command, uint32_t timeout);
static uint8_t ESP_WaitFor(const char *needle, uint32_t timeout);
static void ESP_StartServer(void);
static void ESP_ProcessClient(void);
static void ESP_SendHttpResponse(int id);
static void BME280_InitSensor(void);
static uint8_t BME280_GetValues(double *temperature, double *pressure, double *humidity);

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                     uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;
    return (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(address << 1),
            reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, (uint16_t)len,
            HAL_MAX_DELAY) == HAL_OK) ? BME280_OK : BME280_E_COMM_FAIL;
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *reg_data,
                      uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;
    return (HAL_I2C_Mem_Write(&hi2c1, (uint16_t)(address << 1),
            reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data,
            (uint16_t)len, HAL_MAX_DELAY) == HAL_OK) ?
            BME280_OK : BME280_E_COMM_FAIL;
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
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    UART_Print("\r\nCAT12B - BME280 HTTP JSON server\r\n");
    BME280_InitSensor();
    ESP_StartServer();

    UART_Print("HTTP server ready. Open http://<ESP8266-IP>/ in a browser.\r\n");

    while (1)
        ESP_ProcessClient();
}

static void BME280_InitSensor(void)
{
    dev.intf_ptr = &dev_addr;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;

    if (bme280_init(&dev) != BME280_OK)
    {
        UART_Print("BME280 initialization failed.\r\n");
        Error_Handler();
    }

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_1X;
    dev.settings.osr_t = BME280_OVERSAMPLING_1X;
    dev.settings.filter = BME280_FILTER_COEFF_OFF;
    dev.settings.standby_time = BME280_STANDBY_TIME_1000_MS;

    if (bme280_set_sensor_settings(BME280_SEL_OSR_PRESS |
        BME280_SEL_OSR_TEMP | BME280_SEL_OSR_HUM |
        BME280_SEL_FILTER | BME280_SEL_STANDBY, &dev) != BME280_OK ||
        bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev) != BME280_OK)
    {
        UART_Print("BME280 configuration failed.\r\n");
        Error_Handler();
    }
}

static uint8_t BME280_GetValues(double *temperature, double *pressure, double *humidity)
{
    if (bme280_get_sensor_data(BME280_ALL, &comp_data, &dev) != BME280_OK)
        return 0U;

    *temperature = comp_data.temperature;
    *pressure = comp_data.pressure / 100.0;
    *humidity = comp_data.humidity;
    return 1U;
}

static void ESP_SendCommand(const char *command, uint32_t timeout)
{
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    HAL_UART_Transmit(&huart1, (uint8_t *)command,
                      (uint16_t)strlen(command), 2000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);

    while ((HAL_GetTick() - start) < timeout)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
            HAL_UART_Transmit(&huart2, &ch, 1U, 100U);
    }
}

static uint8_t ESP_WaitFor(const char *needle, uint32_t timeout)
{
    char response[256];
    uint16_t length = 0U;
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    response[0] = '\0';

    while ((HAL_GetTick() - start) < timeout)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
        {
            if (length < sizeof(response) - 1U)
            {
                response[length++] = (char)ch;
                response[length] = '\0';
            }
            HAL_UART_Transmit(&huart2, &ch, 1U, 100U);
            if (strstr(response, needle) != NULL)
                return 1U;
        }
    }
    return 0U;
}

static void ESP_StartServer(void)
{
    char command[160];

    ESP_SendCommand("AT", 1000U);
    ESP_SendCommand("AT+CWMODE=1", 1000U);

    (void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"",
                   WIFI_SSID, WIFI_PASSWORD);
    ESP_SendCommand(command, WIFI_CONNECT_TIMEOUT);

    ESP_SendCommand("AT+CIFSR", 2000U);
    ESP_SendCommand("AT+CIPMUX=1", 1000U);
    ESP_SendCommand("AT+CIPSERVER=1,80", 1000U);
    serverReady = 1U;
}

static void ESP_ProcessClient(void)
{
    uint8_t ch;
    char *ipd;
    char *comma;

    if (serverReady == 0U)
        return;

    if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) != HAL_OK)
        return;

    if (requestLength < sizeof(requestBuffer) - 1U)
        requestBuffer[requestLength++] = (char)ch;
    else
        requestLength = 0U;

    requestBuffer[requestLength] = '\0';

    ipd = strstr(requestBuffer, "+IPD,");
    if (ipd != NULL)
    {
        comma = strchr(ipd + 5, ',');
        if (comma != NULL)
            clientId = atoi(ipd + 5);
    }

    if (strstr(requestBuffer, "\r\n\r\n") != NULL)
    {
        ESP_SendHttpResponse(clientId);
        requestLength = 0U;
        requestBuffer[0] = '\0';
    }
}

static void ESP_SendHttpResponse(int id)
{
    char json[160];
    char httpResponse[320];
    char command[48];
    double temperature, pressure, humidity;
    int jsonLength;
    int responseLength;

    if (!BME280_GetValues(&temperature, &pressure, &humidity))
    {
        (void)strcpy(json, "{\"status\":\"sensor_error\"}");
    }
    else
    {
        (void)snprintf(json, sizeof(json),
            "{\"status\":\"success\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
            temperature, humidity, pressure);
    }

    jsonLength = (int)strlen(json);
    responseLength = snprintf(httpResponse, sizeof(httpResponse),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n%s", jsonLength, json);

    (void)snprintf(command, sizeof(command), "AT+CIPSEND=%d,%d",
                   id, responseLength);
    HAL_UART_Transmit(&huart1, (uint8_t *)command,
                      (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);

    if (ESP_WaitFor(">", 3000U))
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)httpResponse,
                          (uint16_t)responseLength, 3000U);
        (void)ESP_WaitFor("SEND OK", 5000U);
    }

    (void)snprintf(command, sizeof(command), "AT+CIPCLOSE=%d", id);
    HAL_UART_Transmit(&huart1, (uint8_t *)command,
                      (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);
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
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200U;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();
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
