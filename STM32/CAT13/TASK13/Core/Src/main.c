#include "main.h"
#include "app_config.h"
#include "bme280.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static struct bme280_dev sensor;
static struct bme280_data sensorData;
static uint8_t sensorAddress = BME280_I2C_ADDRESS;
static char requestBuffer[900];
static uint16_t requestLength = 0U;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void UART_Print(const char *text);
static uint8_t ESP_SendCommand(const char *command, const char *expected, uint32_t timeout);
static uint8_t ESP_WaitForPrompt(uint32_t timeout);
static void ESP_StartHttpServer(void);
static uint8_t BME280_InitSensor(void);
static uint8_t BME280_ReadJson(char *json, uint16_t size);
static uint8_t HTTP_RequestReady(void);
static int HTTP_GetLinkId(void);
static void HTTP_SendSensorResponse(int linkId);
static void ESP_ProcessIncoming(void);

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                     uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;

    if (HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(address << 1),
                         reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data,
                         (uint16_t)len, HAL_MAX_DELAY) == HAL_OK)
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
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    UART_Print("\r\nCAT13 - BME280 HTTP Sensor Node\r\n");

    if (BME280_InitSensor() == 0U)
    {
        UART_Print("BME280 initialization failed. Check I2C wiring/address.\r\n");
        Error_Handler();
    }

    ESP_StartHttpServer();
    UART_Print("HTTP server ready. Browser and Node-RED can request the sensor JSON.\r\n");

    while (1)
        ESP_ProcessIncoming();
}

static uint8_t BME280_InitSensor(void)
{
    sensor.intf_ptr = &sensorAddress;
    sensor.intf = BME280_I2C_INTF;
    sensor.read = user_i2c_read;
    sensor.write = user_i2c_write;
    sensor.delay_us = user_delay_us;

    if (bme280_init(&sensor) != BME280_OK)
        return 0U;

    sensor.settings.osr_h = BME280_OVERSAMPLING_1X;
    sensor.settings.osr_p = BME280_OVERSAMPLING_1X;
    sensor.settings.osr_t = BME280_OVERSAMPLING_1X;
    sensor.settings.filter = BME280_FILTER_COEFF_OFF;
    sensor.settings.standby_time = BME280_STANDBY_TIME_1000_MS;

    if (bme280_set_sensor_settings(
        BME280_SEL_OSR_PRESS | BME280_SEL_OSR_TEMP |
        BME280_SEL_OSR_HUM | BME280_SEL_FILTER |
        BME280_SEL_STANDBY, &sensor) != BME280_OK)
        return 0U;

    if (bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &sensor) != BME280_OK)
        return 0U;

    UART_Print("BME280 initialized using Bosch SensorAPI.\r\n");
    return 1U;
}

static uint8_t BME280_ReadJson(char *json, uint16_t size)
{
    double temperature;
    double pressure;
    double humidity;

    if (bme280_get_sensor_data(BME280_ALL, &sensorData, &sensor) != BME280_OK)
        return 0U;

    temperature = sensorData.temperature;
    pressure = sensorData.pressure / 100.0;
    humidity = sensorData.humidity;

    (void)snprintf(json, size,
        "{\"status\":\"success\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
        temperature, humidity, pressure);

    return 1U;
}

static uint8_t ESP_SendCommand(const char *command, const char *expected, uint32_t timeout)
{
    char response[256];
    uint16_t length = 0U;
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    HAL_UART_Transmit(&huart1, (uint8_t *)command,
                      (uint16_t)strlen(command), 2000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);

    response[0] = '\0';

    while ((HAL_GetTick() - start) < timeout)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
        {
            HAL_UART_Transmit(&huart2, &ch, 1U, 100U);

            if (length < sizeof(response) - 1U)
            {
                response[length++] = (char)ch;
                response[length] = '\0';
            }

            if (expected != NULL && strstr(response, expected) != NULL)
                return 1U;

            if (strstr(response, "ERROR") != NULL ||
                strstr(response, "FAIL") != NULL)
                return 0U;
        }
    }

    return (expected == NULL) ? 1U : 0U;
}

static void ESP_StartHttpServer(void)
{
    char command[180];

    UART_Print("Checking ESP8266 AT interface...\r\n");
    if (ESP_SendCommand("AT", "OK", 3000U) == 0U)
        UART_Print("No AT response from ESP8266.\r\n");

    (void)ESP_SendCommand("AT+CWMODE=1", "OK", 3000U);

    (void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"",
                   WIFI_SSID, WIFI_PASSWORD);
    UART_Print("Connecting ESP8266 to Wi-Fi...\r\n");
    (void)ESP_SendCommand(command, "WIFI GOT IP", WIFI_CONNECT_TIMEOUT);

    UART_Print("\r\nESP8266 IP address:\r\n");
    (void)ESP_SendCommand("AT+CIFSR", "OK", 3000U);

    (void)ESP_SendCommand("AT+CIPMUX=1", "OK", 3000U);
    (void)ESP_SendCommand("AT+CIPSERVER=1,80", "OK", 3000U);
}

static uint8_t HTTP_RequestReady(void)
{
    char *frame = strstr(requestBuffer, "+IPD,");
    char *colon;

    if (frame == NULL)
        return 0U;

    colon = strchr(frame, ':');
    if (colon == NULL)
        return 0U;

    return (strstr(colon, "\r\n\r\n") != NULL) ? 1U : 0U;
}

static int HTTP_GetLinkId(void)
{
    char *frame = strstr(requestBuffer, "+IPD,");
    int linkId = 0;
    int dataLength = 0;

    if (frame != NULL)
        (void)sscanf(frame, "+IPD,%d,%d:", &linkId, &dataLength);

    (void)dataLength;
    return linkId;
}

static uint8_t ESP_WaitForPrompt(uint32_t timeout)
{
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
        {
            HAL_UART_Transmit(&huart2, &ch, 1U, 100U);
            if (ch == '>')
                return 1U;
        }
    }

    UART_Print("ESP8266 CIPSEND prompt timeout.\r\n");
    return 0U;
}

static void HTTP_SendSensorResponse(int linkId)
{
    char json[180];
    char response[360];
    char command[64];
    int jsonLength;
    int responseLength;

    if (BME280_ReadJson(json, sizeof(json)) == 0U)
        (void)strcpy(json, "{\"status\":\"sensor_error\"}");

    jsonLength = (int)strlen(json);

    responseLength = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n%s", jsonLength, json);

    (void)snprintf(command, sizeof(command), "AT+CIPSEND=%d,%d",
                   linkId, responseLength);

    HAL_UART_Transmit(&huart1, (uint8_t *)command,
                      (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);

    if (ESP_WaitForPrompt(4000U) != 0U)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)response,
                          (uint16_t)responseLength, 3000U);
        (void)ESP_SendCommand("", "SEND OK", 5000U);
        UART_Print("\r\nHTTP sensor JSON sent:\r\n");
        UART_Print(json);
        UART_Print("\r\n");
    }

    (void)snprintf(command, sizeof(command), "AT+CIPCLOSE=%d", linkId);
    (void)ESP_SendCommand(command, "OK", 3000U);
}

static void ESP_ProcessIncoming(void)
{
    uint8_t ch;

    if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) != HAL_OK)
        return;

    if (requestLength < sizeof(requestBuffer) - 1U)
    {
        requestBuffer[requestLength++] = (char)ch;
        requestBuffer[requestLength] = '\0';
    }
    else
    {
        requestLength = 0U;
        requestBuffer[0] = '\0';
        UART_Print("HTTP receive buffer cleared (overflow).\r\n");
        return;
    }

    if (HTTP_RequestReady() != 0U)
    {
        int linkId = HTTP_GetLinkId();
        UART_Print("\r\nHTTP request received:\r\n");
        UART_Print(requestBuffer);
        HTTP_SendSensorResponse(linkId);
        requestLength = 0U;
        requestBuffer[0] = '\0';
    }
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
    huart1.Init.BaudRate = WEMOS_UART_BAUDRATE;
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
