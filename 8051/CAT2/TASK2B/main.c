#include <reg51.h>
#include "delay.h"

/* External LED connected to P1.0 through a current-limiting resistor.
   Active-low wiring: LED anode to +5 V through resistor, cathode to P1.0. */
sbit LED = P1^0;

void main(void)
{
    while (1)
    {
        LED = 0;          /* LED ON: P1.0 sinks current */
        delay_ms(500);

        LED = 1;          /* LED OFF */
        delay_ms(500);
    }
}
