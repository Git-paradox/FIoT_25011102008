#include "main.h"
#include "app_config.h"
#include "bme280.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

static BME280_HandleTypeDef sensor;
static uint8_t sensorReady = 0U;
static uint8_t tcpConnected = 0U;
static char espResponse[1024];
static char pcCommand[128];
static uint16_t pcLength = 0U;
static uint8_t previousWasCR = 0U;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void SendText(UART_HandleTypeDef *uart, const char *text);
static uint16_t ReadESPResponse(char *response, uint16_t size, uint32_t timeout);
static uint8_t SendAT(const char *command, uint32_t timeout);
static uint8_t SendSensorPayload(const char *payload);
static void ConnectWiFi(void);
static void ConnectTCP(void);
static void ReadAndReportSensor(void);
static void ProcessPCCommand(void);
static uint8_t IsConfigured(const char *value, const char *placeholder);

int main(void)
{
    uint8_t ch;
    uint32_t lastReport;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();

    SendText(&huart2, "\r\nCAT10 - STM32 / Wemos ESP8266\r\n");
    SendText(&huart2, "UART1 Wemos: 115200 8N1 | UART2 PC: 9600 8N1\r\n");
    SendText(&huart2, "Commands: WIFI, SENSOR, STATUS or any AT command\r\n");

    if (BME280_Init(&sensor, &hi2c1) == HAL_OK)
    {
        sensorReady = 1U;
        SendText(&huart2, "BME280 detected on I2C1 (PB6=SCL, PB7=SDA)\r\n");
    }
    else
        SendText(&huart2, "BME280 not detected. Check wiring and address.\r\n");

    HAL_Delay(1800U);
    (void)SendAT("AT", 3000U);
    (void)SendAT("AT+GMR", 4000U);
    (void)SendAT("AT+CWMODE=1", 4000U);
    (void)SendAT("AT+CWLAP", 15000U);

    if (IsConfigured(WIFI_SSID, "YOUR_WIFI_SSID") &&
        IsConfigured(WIFI_PASSWORD, "YOUR_WIFI_PASSWORD"))
    {
        ConnectWiFi();
        ConnectTCP();
    }
    else
    {
        SendText(&huart2, "\r\nSet Wi-Fi values in Core/Inc/app_config.h and rebuild.\r\n");
        SendText(&huart2, "TCP sending also needs a configured server and listening port.\r\n");
    }

    lastReport = HAL_GetTick();

    while (1)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1U, 10U) == HAL_OK)
        {
            HAL_UART_Transmit(&huart2, &ch, 1U, 100U);

            if (ch == '\r')
            {
                ProcessPCCommand();
                previousWasCR = 1U;
            }
            else if (ch == '\n')
            {
                if (previousWasCR == 0U)
                    ProcessPCCommand();
                previousWasCR = 0U;
            }
            else
            {
                previousWasCR = 0U;
                if (pcLength < sizeof(pcCommand) - 1U)
                    pcCommand[pcLength++] = (char)ch;
                else
                {
                    pcLength = 0U;
                    SendText(&huart2, "\r\nInput too long.\r\n");
                }
            }
        }

        if ((HAL_GetTick() - lastReport) >= SENSOR_REPORT_INTERVAL)
        {
            lastReport = HAL_GetTick();
            ReadAndReportSensor();
        }
    }
}

static void SendText(UART_HandleTypeDef *uart, const char *text)
{
    HAL_UART_Transmit(uart, (uint8_t *)text, (uint16_t)strlen(text), HAL_MAX_DELAY);
}

static uint16_t ReadESPResponse(char *response, uint16_t size, uint32_t timeout)
{
    uint8_t ch;
    uint16_t length = 0U;
    uint32_t start = HAL_GetTick();
    uint32_t last = start;
    uint8_t gotData = 0U;

    while ((HAL_GetTick() - start) < timeout)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
        {
            if (length < size - 1U)
                response[length++] = (char)ch;
            HAL_UART_Transmit(&huart2, &ch, 1U, 100U);
            gotData = 1U;
            last = HAL_GetTick();
        }
        else if (gotData && ((HAL_GetTick() - last) > 350U))
            break;
    }

    response[length] = '\0';
    if (!gotData)
        SendText(&huart2, "\r\n[No response from Wemos]\r\n");
    return length;
}

static uint8_t SendAT(const char *command, uint32_t timeout)
{
    const uint8_t ending[2] = {'\r', '\n'};

    SendText(&huart2, "\r\n>> ");
    SendText(&huart2, command);
    SendText(&huart2, "\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)command, (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)ending, 2U, 1000U);
    (void)ReadESPResponse(espResponse, sizeof(espResponse), timeout);

    return ((strstr(espResponse, "OK") != NULL) ||
            (strstr(espResponse, "CONNECT") != NULL) ||
            (strstr(espResponse, "WIFI GOT IP") != NULL)) ? 1U : 0U;
}

static uint8_t IsConfigured(const char *value, const char *placeholder)
{
    return ((value[0] != '\0') && (strcmp(value, placeholder) != 0)) ? 1U : 0U;
}

static void ConnectWiFi(void)
{
    char command[160];

    SendText(&huart2, "\r\nJoining the configured Wi-Fi network...\r\n");
    (void)SendAT("AT+CWMODE=1", 4000U);
    (void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"",
                   WIFI_SSID, WIFI_PASSWORD);
    (void)SendAT(command, WIFI_CONNECT_TIMEOUT);
    (void)SendAT("AT+CIFSR", 5000U);
    (void)SendAT("AT+CWSTATE?", 4000U);
}

static void ConnectTCP(void)
{
    char command[160];

    tcpConnected = 0U;
    if (!IsConfigured(TCP_SERVER_IP, "YOUR_SERVER_IP"))
    {
        SendText(&huart2, "\r\nTCP server IP is not set. Sensor readings will remain on the PC terminal.\r\n");
        return;
    }

    (void)snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\",\"%s\",%u",
                   TCP_SERVER_IP, (unsigned int)TCP_SERVER_PORT);
    if (SendAT(command, TCP_CONNECT_TIMEOUT))
    {
        tcpConnected = 1U;
        (void)SendAT("AT+CIPSTATUS", 4000U);
    }
    else
        SendText(&huart2, "\r\nTCP connection not confirmed. Check server IP/port.\r\n");
}

static uint8_t SendSensorPayload(const char *payload)
{
    char command[48];
    uint16_t length = (uint16_t)strlen(payload);

    (void)snprintf(command, sizeof(command), "AT+CIPSEND=%u", (unsigned int)length);
    SendText(&huart2, "\r\n>> ");
    SendText(&huart2, command);
    SendText(&huart2, "\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)command, (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);
    (void)ReadESPResponse(espResponse, sizeof(espResponse), 5000U);

    if (strchr(espResponse, '>') == NULL)
    {
        SendText(&huart2, "\r\nWemos did not provide the data prompt.\r\n");
        return 0U;
    }

    HAL_UART_Transmit(&huart1, (uint8_t *)payload, length, 3000U);
    (void)ReadESPResponse(espResponse, sizeof(espResponse), 6000U);
    return (strstr(espResponse, "SEND OK") != NULL) ? 1U : 0U;
}

static void ReadAndReportSensor(void)
{
    BME280_Data data;
    char payload[100];
    int32_t absTemp;
    uint32_t humidityWhole;
    uint32_t humidityDecimal;

    if (!sensorReady)
    {
        SendText(&huart2, "\r\nBME280 unavailable. Check the sensor wiring.\r\n");
        return;
    }

    if (BME280_Read(&sensor, &data) != HAL_OK)
    {
        SendText(&huart2, "\r\nBME280 read failed.\r\n");
        return;
    }

    absTemp = (data.temperatureCentiC < 0) ? -data.temperatureCentiC : data.temperatureCentiC;
    humidityWhole = data.humidityCentiPercent / 100U;
    humidityDecimal = data.humidityCentiPercent % 100U;

    (void)snprintf(payload, sizeof(payload),
                   "T=%s%ld.%02ldC,P=%luPa,H=%lu.%02lu%%\r\n",
                   (data.temperatureCentiC < 0) ? "-" : "",
                   (long)(absTemp / 100), (long)(absTemp % 100),
                   (unsigned long)data.pressurePa,
                   (unsigned long)humidityWhole, (unsigned long)humidityDecimal);

    SendText(&huart2, "\r\nSensor: ");
    SendText(&huart2, payload);

    if (tcpConnected)
        (void)SendSensorPayload(payload);
}

static void ProcessPCCommand(void)
{
    uint32_t timeout = 5000U;

    pcCommand[pcLength] = '\0';
    if (pcLength == 0U)
        return;

    SendText(&huart2, "\r\n");

    if (strcmp(pcCommand, "SENSOR") == 0)
        ReadAndReportSensor();
    else if (strcmp(pcCommand, "WIFI") == 0)
    {
        ConnectWiFi();
        ConnectTCP();
    }
    else if (strcmp(pcCommand, "STATUS") == 0)
    {
        (void)SendAT("AT+CIFSR", 5000U);
        (void)SendAT("AT+CIPSTATUS", 5000U);
    }
    else if (strncmp(pcCommand, "AT", 2U) == 0)
    {
        if (strncmp(pcCommand, "AT+CWLAP", 8U) == 0)
            timeout = 15000U;
        else if (strncmp(pcCommand, "AT+CWJAP", 8U) == 0)
            timeout = WIFI_CONNECT_TIMEOUT;
        else if (strncmp(pcCommand, "AT+CIPSTART", 11U) == 0)
            timeout = TCP_CONNECT_TIMEOUT;
        (void)SendAT(pcCommand, timeout);
    }
    else
        SendText(&huart2, "Use an AT command, WIFI, SENSOR or STATUS.\r\n");

    pcLength = 0U;
}

void SystemClock_Config(void)
{
    /* Use the STM32F401RE internal 16 MHz HSI clock. */
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
    huart2.Init.BaudRate = PC_UART_BAUDRATE;
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
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        for (volatile uint32_t delay = 0U; delay < 250000U; delay++)
        {
        }
    }
}
