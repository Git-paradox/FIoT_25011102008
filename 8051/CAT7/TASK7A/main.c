#include <REGX51.H>
#include "lcd.h"

void main(void)
{
    lcd_init();

    lcd_goto(0, 0);
    lcd_data('A');

    lcd_goto(1, 0);
    lcd_puts("LCD Ready");

    while (1)
    {
        ;
    }
}
