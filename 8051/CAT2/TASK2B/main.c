#ifdef __SDCC
__sfr __at (0x90) P1;
__sbit __at (0x90) LED;
#else
#include <reg51.h>
sbit LED = P1^0;
#endif

#include "delay.h"

/* External LED: +5 V -> 330 ohm resistor -> LED anode;
   LED cathode -> P1.0. This is an active-low connection. */
void main(void)
{
    while (1)
    {
        LED = 0;
        delay_ms(500);

        LED = 1;
        delay_ms(500);
    }
}
