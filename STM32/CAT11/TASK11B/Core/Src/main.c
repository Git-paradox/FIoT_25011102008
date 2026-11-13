#include "main.h"

#define LED_PORT GPIOA
#define LED_PIN GPIO_PIN_5
#define BUTTON_PORT GPIOC
#define BUTTON_PIN GPIO_PIN_13
#define DEBOUNCE_MS 25U
#define LONG_PRESS_MS 1000U
#define BLINK_INTERVAL_MS 250U

static GPIO_PinState lastRawState = GPIO_PIN_SET;
static GPIO_PinState stableState = GPIO_PIN_SET;
static uint32_t lastChangeTime = 0U;
static uint32_t pressStartTime = 0U;
static uint32_t lastBlinkTime = 0U;
static uint8_t ledState = 0U;
static uint8_t blinkMode = 0U;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int main(void)
{
    GPIO_PinState rawState;
    uint32_t now;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    while (1)
    {
        now = HAL_GetTick();
        rawState = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

        if (rawState != lastRawState)
        {
            lastRawState = rawState;
            lastChangeTime = now;
        }

        if ((now - lastChangeTime) >= DEBOUNCE_MS &&
            stableState != lastRawState)
        {
            stableState = lastRawState;

            if (stableState == GPIO_PIN_RESET)
            {
                pressStartTime = now;
            }
            else
            {
                uint32_t pressDuration = now - pressStartTime;

                if (pressDuration >= LONG_PRESS_MS)
                {
                    blinkMode = !blinkMode;
                    lastBlinkTime = now;
                    if (blinkMode == 0U)
                    {
                        ledState = 0U;
                        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
                    }
                }
                else
                {
                    blinkMode = 0U;
                    ledState = !ledState;
                    HAL_GPIO_WritePin(LED_PORT, LED_PIN,
                                      ledState ? GPIO_PIN_SET : GPIO_PIN_RESET);
                }
            }
        }

        if (blinkMode != 0U &&
            (now - lastBlinkTime) >= BLINK_INTERVAL_MS)
        {
            lastBlinkTime = now;
            HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        }
    }
}

void SystemClock_Config(void)
{
    /* Use the internal 16 MHz HSI clock. */
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
