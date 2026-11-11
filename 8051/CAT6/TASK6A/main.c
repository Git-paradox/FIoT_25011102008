#include <REGX51.H>
#include "uart.h"

volatile unsigned int system_ms = 0;

void timer0_isr(void) interrupt 1
{
    TH0 = 0xFC;
    TL0 = 0x66;
    system_ms++;
}

void main(void)
{
    unsigned int now;
    unsigned int last_message = 0;
    unsigned char received_char;

    EA = 0;

    /* Timer 0: 1 ms system time base. */
    TMOD = (TMOD & 0xF0) | 0x01;
    TH0 = 0xFC;
    TL0 = 0x66;

    uart_init();

    ET0 = 1;
    TR0 = 1;
    EA = 1;

    uart_puts("UART Ready - 9600 8N1\r\n");
    uart_puts("Hello from 8051\r\n");

    while (1)
    {
        ET0 = 0;
        now = system_ms;
        ET0 = 1;

        /* Task A: send the message continuously, once every second. */
        if ((unsigned int)(now - last_message) >= 1000)
        {
            last_message = now;
            uart_puts("Hello from 8051\r\n");
        }

        /* Task B: echo every character received from the PC terminal. */
        if (uart_available() != 0)
        {
            received_char = uart_getc();
            uart_putc(received_char);
        }
    }
}
