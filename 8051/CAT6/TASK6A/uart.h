#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_putc(unsigned char data);
void uart_puts(unsigned char code *text);
unsigned char uart_available(void);
unsigned char uart_getc(void);
unsigned char uart_overflow_count(void);

#endif
