#include "main.h"
#include <string.h>

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static char commandBuffer[128];
static uint16_t commandLength = 0U;
static char responseBuffer[512];
static uint8_t previousWasCR = 0U;

void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void SendPCText(const char *text);
static void ProcessCommand(void);
static void ReadResponse(void);

int main(void)
{
    uint8_t received;

    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    SendPCText("\r\nCAT10B - ESP8266 AT command interface\r\n");
    SendPCText("Enter an AT command and press Enter.\r\n");

    while (1)
    {
        if (HAL_UART_Receive(&huart2, &received, 1U, 10U) == HAL_OK)
        {
            HAL_UART_Transmit(&huart2, &received, 1U, HAL_MAX_DELAY);

            if (received == '\r')
            {
                ProcessCommand();
                previousWasCR = 1U;
            }
            else if (received == '\n')
            {
                if (previousWasCR == 0U)
                    ProcessCommand();
                previousWasCR = 0U;
            }
            else
            {
                previousWasCR = 0U;
                if (commandLength < sizeof(commandBuffer) - 1U)
                    commandBuffer[commandLength++] = (char)received;
                else
                {
                    commandLength = 0U;
                    SendPCText("\r\nCommand too long.\r\n");
                }
            }
        }
    }
}

static void SendPCText(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text,
                      (uint16_t)strlen(text), HAL_MAX_DELAY);
}

static void ProcessCommand(void)
{
    if (commandLength == 0U)
        return;

    commandBuffer[commandLength] = '\0';
    SendPCText("\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)commandBuffer,
                      commandLength, 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);
    ReadResponse();
    commandLength = 0U;
}

static void ReadResponse(void)
{
    uint8_t ch;
    uint16_t length = 0U;
    uint32_t start = HAL_GetTick();
    uint32_t last = start;
    uint8_t receivedAny = 0U;

    while ((HAL_GetTick() - start) < 12000U)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
        {
            if (length < sizeof(responseBuffer) - 1U)
                responseBuffer[length++] = (char)ch;
            HAL_UART_Transmit(&huart2, &ch, 1U, HAL_MAX_DELAY);
            last = HAL_GetTick();
            receivedAny = 1U;
        }
        else if (receivedAny && ((HAL_GetTick() - last) > 400U))
        {
            break;
        }
    }

    responseBuffer[length] = '\0';
    if (receivedAny == 0U)
        SendPCText("\r\n[No response from Wemos]\r\n");
}

void SystemClock_Config(void)
{
    /* Use the internal 16 MHz HSI clock. */
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

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
