#include "main.h"

UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);

int main(void)
{
    uint8_t received;
    uint8_t transmitCharacter = 'A';
    uint32_t lastTransmitTime;

    HAL_Init();
    SystemClock_Config();
    MX_USART2_UART_Init();

    HAL_UART_Transmit(&huart2,
                      (uint8_t *)"\r\nUART communication ready\r\n",
                      28U, HAL_MAX_DELAY);

    lastTransmitTime = HAL_GetTick();

    while (1)
    {
        if (HAL_UART_Receive(&huart2, &received, 1U, 10U) == HAL_OK)
            HAL_UART_Transmit(&huart2, &received, 1U, HAL_MAX_DELAY);

        if ((HAL_GetTick() - lastTransmitTime) >= 1000U)
        {
            lastTransmitTime = HAL_GetTick();
            HAL_UART_Transmit(&huart2, &transmitCharacter, 1U, HAL_MAX_DELAY);
        }
    }
}

void SystemClock_Config(void)
{
    /* Use the internal 16 MHz HSI clock. */
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
