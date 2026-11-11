#include "task8_variants.h"
#include "main.h"

/* Task 8B: one LED toggle per valid debounced button press.
   This is a non-blocking polling implementation. */

#define TASK8B_DEBOUNCE_MS 25U

static GPIO_PinState task8bRawLast = GPIO_PIN_SET;
static GPIO_PinState task8bStable = GPIO_PIN_SET;
static uint32_t task8bLastChange = 0U;

void Task8B_Update(void)
{
    GPIO_PinState rawState;
    uint32_t now;

    now = HAL_GetTick();
    rawState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    if (rawState != task8bRawLast)
    {
        task8bRawLast = rawState;
        task8bLastChange = now;
    }

    if ((now - task8bLastChange) >= TASK8B_DEBOUNCE_MS)
    {
        if (task8bStable != task8bRawLast)
        {
            task8bStable = task8bRawLast;

            if (task8bStable == GPIO_PIN_RESET)
            {
                HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            }
        }
    }
}
