#include "main.h"

#define LED_GPIO_Port       GPIOA
#define LED_Pin             GPIO_PIN_5
#define BUTTON_GPIO_Port    GPIOC
#define BUTTON_Pin          GPIO_PIN_13

#define BUTTON_PRESSED      GPIO_PIN_RESET
#define DEBOUNCE_TIME_MS    25U
#define LONG_PRESS_TIME_MS  1000U
#define BLINK_INTERVAL_MS   250U

typedef enum
{
    APP_MODE_STATIC = 0,
    APP_MODE_BLINKING
} AppMode_t;

static AppMode_t appMode = APP_MODE_STATIC;
static GPIO_PinState lastRawButton = GPIO_PIN_SET;
static GPIO_PinState stableButton = GPIO_PIN_SET;

static uint32_t rawChangeTime = 0U;
static uint32_t pressStartTime = 0U;
static uint32_t blinkChangeTime = 0U;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void Application_Update(uint32_t currentTime);

int main(void)
{
    uint32_t currentTime;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    while (1)
    {
        currentTime = HAL_GetTick();

        Application_Update(currentTime);

        if (appMode == APP_MODE_BLINKING)
        {
            if ((currentTime - blinkChangeTime) >= BLINK_INTERVAL_MS)
            {
                blinkChangeTime = currentTime;
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
        }
    }
}

static void Application_Update(uint32_t currentTime)
{
    GPIO_PinState rawButton;
    uint32_t pressDuration;

    rawButton = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);

    if (rawButton != lastRawButton)
    {
        lastRawButton = rawButton;
        rawChangeTime = currentTime;
    }

    if ((currentTime - rawChangeTime) >= DEBOUNCE_TIME_MS)
    {
        if (stableButton != lastRawButton)
        {
            stableButton = lastRawButton;

            if (stableButton == BUTTON_PRESSED)
            {
                pressStartTime = currentTime;
            }
            else
            {
                pressDuration = currentTime - pressStartTime;

                if (pressDuration >= LONG_PRESS_TIME_MS)
                {
                    appMode = APP_MODE_BLINKING;
                    blinkChangeTime = currentTime;
                    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
                }
                else
                {
                    if (appMode == APP_MODE_BLINKING)
                    {
                        appMode = APP_MODE_STATIC;
                        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
                    }
                    else
                    {
                        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
                    }
                }
            }
        }
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
    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BUTTON_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}
