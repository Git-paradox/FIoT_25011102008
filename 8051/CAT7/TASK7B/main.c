#include <REGX51.H>
#include "lcd.h"

void delay_ms(unsigned int milliseconds)
{
    unsigned int i;
    unsigned char j;

    for (i = 0; i < milliseconds; i++)
    {
        for (j = 0; j < 120; j++)
        {
            ;
        }
    }
}

void main(void)
{
    unsigned int value = 0;

    lcd_init();

    lcd_goto(0, 0);
    lcd_puts("Dynamic Value");

    while (1)
    {
        lcd_goto(1, 0);
        lcd_puts("Value: ");
        lcd_print_number(value);
        lcd_puts("    ");

        value++;
        if (value > 9999)
            value = 0;

        delay_ms(500);
    }
}
