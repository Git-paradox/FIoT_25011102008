#include <REGX51.H>
#include "lcd.h"

unsigned char code row_mask[4] = {0xFE, 0xFD, 0xFB, 0xF7};

unsigned char code key_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void keypad_delay_ms(unsigned int milliseconds)
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

unsigned char keypad_scan(void)
{
    unsigned char row;
    unsigned char columns;
    unsigned char column;

    P1 = 0xFF;

    for (row = 0; row < 4; row++)
    {
        P1 = row_mask[row];
        columns = P1 & 0xF0;

        if (columns != 0xF0)
        {
            if ((columns & 0x10) == 0)
                column = 0;
            else if ((columns & 0x20) == 0)
                column = 1;
            else if ((columns & 0x40) == 0)
                column = 2;
            else
                column = 3;

            P1 = 0xFF;
            return key_map[row][column];
        }
    }

    P1 = 0xFF;
    return 0xFF;
}

void move_character(unsigned char key, unsigned char *row, unsigned char *column)
{
    if (key == '2' && *row > 0)
        (*row)--;
    else if (key == '8' && *row < 1)
        (*row)++;
    else if (key == '4' && *column > 0)
        (*column)--;
    else if (key == '6' && *column < 15)
        (*column)++;
}

void main(void)
{
    unsigned char key;
    unsigned char row = 0;
    unsigned char column = 0;

    lcd_init();

    lcd_goto(0, 0);
    lcd_data('A');

    while (1)
    {
        key = keypad_scan();

        if (key != 0xFF)
        {
            keypad_delay_ms(20);

            if (keypad_scan() == key)
            {
                if ((key == '2') || (key == '8') ||
                    (key == '4') || (key == '6'))
                {
                    lcd_goto(row, column);
                    lcd_data(' ');

                    move_character(key, &row, &column);

                    lcd_goto(row, column);
                    lcd_data('A');
                }

                while (keypad_scan() != 0xFF)
                    keypad_delay_ms(5);
            }
        }
    }
}
