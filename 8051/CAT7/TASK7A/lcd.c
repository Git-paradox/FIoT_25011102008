#include <REGX51.H>
#include "lcd.h"

/* HD44780-compatible LCD in 4-bit mode.
   RS=P2.0, E=P2.1, D4-D7=P2.4-P2.7.
   RW is connected to GND. */

sbit LCD_RS = P2^0;
sbit LCD_EN = P2^1;
sbit LCD_D4 = P2^4;
sbit LCD_D5 = P2^5;
sbit LCD_D6 = P2^6;
sbit LCD_D7 = P2^7;

void lcd_delay_ms(unsigned int milliseconds)
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

void lcd_pulse_enable(void)
{
    LCD_EN = 1;
    lcd_delay_ms(1);
    LCD_EN = 0;
    lcd_delay_ms(1);
}

void lcd_write_nibble(unsigned char value)
{
    LCD_D4 = (value & 0x01) ? 1 : 0;
    LCD_D5 = (value & 0x02) ? 1 : 0;
    LCD_D6 = (value & 0x04) ? 1 : 0;
    LCD_D7 = (value & 0x08) ? 1 : 0;

    lcd_pulse_enable();
}

void lcd_write_byte(unsigned char value)
{
    lcd_write_nibble(value >> 4);
    lcd_write_nibble(value & 0x0F);
}

void lcd_cmd(unsigned char command)
{
    LCD_RS = 0;
    lcd_write_byte(command);

    if ((command == 0x01) || (command == 0x02))
        lcd_delay_ms(2);
}

void lcd_data(unsigned char character)
{
    LCD_RS = 1;
    lcd_write_byte(character);
    LCD_RS = 0;
}

void lcd_init(void)
{
    LCD_RS = 0;
    LCD_EN = 0;
    lcd_delay_ms(20);

    lcd_write_nibble(0x03);
    lcd_delay_ms(5);
    lcd_write_nibble(0x03);
    lcd_delay_ms(1);
    lcd_write_nibble(0x03);
    lcd_delay_ms(1);
    lcd_write_nibble(0x02);

    lcd_cmd(0x28);    /* 4-bit, 2-line, 5x8 font */
    lcd_cmd(0x0C);    /* Display ON, cursor OFF */
    lcd_cmd(0x06);    /* Increment cursor, no display shift */
    lcd_cmd(0x01);    /* Clear display */
}

void lcd_goto(unsigned char row, unsigned char column)
{
    unsigned char address;

    if (row == 0)
        address = 0x80 + column;
    else
        address = 0xC0 + column;

    lcd_cmd(address);
}

void lcd_puts(char code *text)
{
    while (*text != 0)
    {
        lcd_data(*text);
        text++;
    }
}

void lcd_print_number(unsigned int value)
{
    unsigned char digits[5];
    unsigned char count = 0;

    if (value == 0)
    {
        lcd_data('0');
        return;
    }

    while ((value > 0) && (count < 5))
    {
        digits[count] = (unsigned char)(value % 10);
        value = value / 10;
        count++;
    }

    while (count > 0)
    {
        count--;
        lcd_data((unsigned char)('0' + digits[count]));
    }
}
