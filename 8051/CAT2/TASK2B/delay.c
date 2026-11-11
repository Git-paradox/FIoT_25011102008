#include <reg51.h>
#include "delay.h"

/* Software delay calibrated approximately for a classic 12-clock 8051
   running from an 11.0592 MHz crystal. Exact timing depends on compiler
   version and optimization settings; verify on the target hardware. */
void delay_ms(unsigned int milliseconds)
{
    unsigned int i;
    unsigned char j;

    for (i = 0; i < milliseconds; i++)
    {
        for (j = 0; j < 123; j++)
        {
            ;
        }
    }
}
