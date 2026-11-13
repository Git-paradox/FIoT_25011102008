#include "main.h"
#include <string.h>

#define LED_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_5
#define COMMAND_SIZE 32U

UART_HandleTypeDef huart2;
static uint8_t commandBuffer[COMMAND_SIZE];
static uint8_t commandLength = 0U;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void UART_SendString(const char *text);
static uint8_t Command_Is(const char *expected);
static void Process_Command(void);

int main(void)
{
    uint8_t received;
    uint8_t transmitCharacter = 'A';
    uint32_t lastTransmitTime;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    UART_SendString("\r\nSTM32 UART READY - 9600 8N1\r\n");
    UART_SendString("Commands: LED ON, LED OFF, LED TOGGLE\r\n");
    lastTransmitTime = HAL_GetTick();

    while (1)
    {
        if (HAL_UART_Receive(&huart2, &received, 1U, 10U) == HAL_OK)
        {
            HAL_UART_Transmit(&huart2, &received, 1U, HAL_MAX_DELAY);

            if ((received == '\r') || (received == '\n'))
            {
                if (commandLength > 0U)
                {
                    commandBuffer[commandLength] = '\0';
                    Process_Command();
                    commandLength = 0U;
                }
            }
            else if ((received == 0x08U) || (received == 0x7FU))
            {
                if (commandLength > 0U)
                {
                    commandLength--;
                    UART_SendString("\b \b");
                }
            }
            else if (commandLength < (COMMAND_SIZE - 1U))
            {
                commandBuffer[commandLength++] = received;
            }
            else
            {
                commandLength = 0U;
                UART_SendString("\r\nCommand too long. Try again.\r\n");
            }
        }

        if ((HAL_GetTick() - lastTransmitTime) >= 1000U)
        {
            lastTransmitTime = HAL_GetTick();
            HAL_UART_Transmit(&huart2, &transmitCharacter, 1U, HAL_MAX_DELAY);
        }
    }
}

static void UART_SendString(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text,
                      (uint16_t)strlen(text), HAL_MAX_DELAY);
}

static uint8_t Command_Is(const char *expected)
{
    uint8_t index = 0U;
    char character;

    while ((expected[index] != '\0') && (index < commandLength))
    {
        character = (char)commandBuffer[index];

        if ((character >= 'a') && (character <= 'z'))
            character = (char)(character - ('a' - 'A'));

        if (character != expected[index])
            return 0U;

        index++;
    }

    return ((expected[index] == '\0') && (index == commandLength)) ? 1U : 0U;
}

static void Process_Command(void)
{
    if (Command_Is("LED ON"))
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        UART_SendString("\r\nLED turned ON\r\n");
    }
    else if (Command_Is("LED OFF"))
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        UART_SendString("\r\nLED turned OFF\r\n");
    }
    else if (Command_Is("LED TOGGLE"))
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        UART_SendString("\r\nLED toggled\r\n");
    }
    else
    {
        UART_SendString("\r\nUnknown command. Use LED ON, LED OFF or LED TOGGLE.\r\n");
    }
}

void SystemClock_Config(void)
{
    /* Use the STM32F401RE internal 16 MHz HSI clock. */
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
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
