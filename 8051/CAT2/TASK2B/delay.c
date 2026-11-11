#include "delay.h"

/* Approximate software delay for an 11.0592 MHz, 12-clock 8051.
   Volatile counters prevent the compiler from removing the empty loops.
   Exact timing depends on compiler version and optimization settings. */
void delay_ms(unsigned int milliseconds)
{
    volatile unsigned int i;
    volatile unsigned char j;

    for (i = 0; i < milliseconds; i++)
    {
        for (j = 0; j < 230; j++)
        {
            ;
        }
    }
}
