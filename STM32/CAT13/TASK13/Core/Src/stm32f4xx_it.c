#include "main.h"
#include "stm32f4xx_it.h"
#include "ESPLib.h"

void SysTick_Handler(void)
{
    MQTTTimer();
    HAL_IncTick();
}
