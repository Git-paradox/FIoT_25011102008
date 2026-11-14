#include "main.h"
#include "app_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static char requestBuffer[768];
static uint16_t requestLength = 0U;

void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void UART_Print(const char *text);
static uint8_t ESP_SendCommand(const char *command, const char *expected, uint32_t timeout);
static uint8_t ESP_WaitForText(const char *expected, uint32_t timeout)
{
    char response[160];
    uint16_t length = 0U;
    uint8_t ch;
    uint32_t start = HAL_GetTick();

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

            if (strstr(response, expected) != NULL)
                return 1U;

            if (strstr(response, "ERROR") != NULL ||
                strstr(response, "FAIL") != NULL)
                return 0U;
        }
    }

    return 0U;
}

static uint8_t ESP_WaitForPrompt(uint32_t timeout);
static void ESP_StartServer(void);
static uint8_t HTTP_RequestReady(void);
static int HTTP_GetLinkId(void);
static void HTTP_SendStaticResponse(int linkId);
static void ESP_ProcessIncoming(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    UART_Print("\r\nCAT11 - STM32/Wemos HTTP server\r\n");
    ESP_StartServer();
    UART_Print("HTTP server is ready. Open the ESP8266 IP address in a browser.\r\n");

    while (1)
        ESP_ProcessIncoming();
}

static uint8_t ESP_SendCommand(const char *command, const char *expected, uint32_t timeout)
{
    char response[256];
    uint16_t length = 0U;
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    HAL_UART_Transmit(&huart1, (uint8_t *)command, (uint16_t)strlen(command), 2000U);
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

static void ESP_StartServer(void)
{
    char command[160];

    (void)ESP_SendCommand("AT", "OK", 3000U);
    (void)ESP_SendCommand("AT+CWMODE=1", "OK", 3000U);

    (void)snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"",
                   WIFI_SSID, WIFI_PASSWORD);
    UART_Print("Connecting to Wi-Fi...\r\n");
    (void)ESP_SendCommand(command, "WIFI GOT IP", WIFI_CONNECT_TIMEOUT);

    UART_Print("\r\nESP8266 network information:\r\n");
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

    UART_Print("ESP8266 did not return the CIPSEND prompt.\r\n");
    return 0U;
}

static void HTTP_SendStaticResponse(int linkId)
{
    static const char body[] =
        "<html><body><h1>STM32 IoT Node</h1>"
        "<p>Hello from STM32</p></body></html>";
    char response[320];
    char command[64];
    int responseLength;

    responseLength = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n\r\n%s",
        (unsigned int)(sizeof(body) - 1U), body);

    (void)snprintf(command, sizeof(command), "AT+CIPSEND=%d,%d", linkId, responseLength);
    HAL_UART_Transmit(&huart1, (uint8_t *)command, (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);

    if (ESP_WaitForPrompt(4000U) != 0U)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)response,
                          (uint16_t)responseLength, 3000U);
        (void)ESP_WaitForText("SEND OK", 5000U);
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
        UART_Print("HTTP request buffer cleared (overflow).\r\n");
        return;
    }

    if (HTTP_RequestReady() != 0U)
    {
        int linkId = HTTP_GetLinkId();
        UART_Print("\r\nHTTP request received:\r\n");
        UART_Print(requestBuffer);
        HTTP_SendStaticResponse(linkId);
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

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
