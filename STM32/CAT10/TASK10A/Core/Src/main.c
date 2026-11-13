#include "main.h"
#include <string.h>

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void SendPCText(const char *text);
static void SendWemosCommand(const char *command);
static void DisplayWemosResponse(uint32_t timeout);

int main(void)
{
    uint8_t ch;

    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    SendPCText("\r\nCAT10A - STM32 Wemos UART test\r\n");
    SendWemosCommand("AT");
    DisplayWemosResponse(3000U);

    while (1)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 10U) == HAL_OK)
            HAL_UART_Transmit(&huart2, &ch, 1U, HAL_MAX_DELAY);

        if (HAL_UART_Receive(&huart2, &ch, 1U, 10U) == HAL_OK)
            HAL_UART_Transmit(&huart1, &ch, 1U, HAL_MAX_DELAY);
    }
}

static void SendPCText(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text,
                      (uint16_t)strlen(text), HAL_MAX_DELAY);
}

static void SendWemosCommand(const char *command)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)command,
                      (uint16_t)strlen(command), 1000U);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2U, 1000U);
}

static void DisplayWemosResponse(uint32_t timeout)
{
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < timeout)
    {
        if (HAL_UART_Receive(&huart1, &ch, 1U, 20U) == HAL_OK)
            HAL_UART_Transmit(&huart2, &ch, 1U, HAL_MAX_DELAY);
    }
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
