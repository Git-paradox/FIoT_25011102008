#ifndef LCD_H
#define LCD_H

void lcd_init(void);
void lcd_cmd(unsigned char command);
void lcd_data(unsigned char character);
void lcd_goto(unsigned char row, unsigned char column);
void lcd_puts(char code *text);
void lcd_print_number(unsigned int value);

#endif
