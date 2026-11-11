#include "task8_variants.h"
#include "main.h"

/* Task 8A: LED follows the current push-button state.
   Call this function repeatedly from a main loop. */

void Task8A_Update(void)
{
    GPIO_PinState buttonState;

    buttonState = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

    if (buttonState == GPIO_PIN_RESET)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
}
