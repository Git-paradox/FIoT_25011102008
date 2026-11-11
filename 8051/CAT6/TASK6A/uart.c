#include <REGX51.H>
#include "uart.h"

#define UART_BUFFER_SIZE 32
#define UART_BUFFER_MASK (UART_BUFFER_SIZE - 1)

/* Interrupt-driven receive and transmit ring buffers. */
volatile unsigned char rx_buffer[UART_BUFFER_SIZE];
volatile unsigned char tx_buffer[UART_BUFFER_SIZE];

volatile unsigned char rx_head = 0;
volatile unsigned char rx_tail = 0;
volatile unsigned char tx_head = 0;
volatile unsigned char tx_tail = 0;

volatile unsigned char tx_active = 0;
volatile unsigned char rx_overflow = 0;

/* UART mode 1, 9600 baud, 8 data bits, no parity, 1 stop bit.
   Timer 1 mode 2 with TH1 = FD assumes an 11.0592 MHz crystal. */
void uart_init(void)
{
    unsigned char i;

    P3 |= 0x03;
    SCON = 0x50;
    PCON &= 0x7F;

    TMOD = (TMOD & 0x0F) | 0x20;
    TH1 = 0xFD;
    TL1 = 0xFD;

    RI = 0;
    TI = 0;

    rx_head = 0;
    rx_tail = 0;
    tx_head = 0;
    tx_tail = 0;
    tx_active = 0;
    rx_overflow = 0;

    for (i = 0; i < UART_BUFFER_SIZE; i++)
    {
        rx_buffer[i] = 0;
        tx_buffer[i] = 0;
    }

    TR1 = 1;
    ES = 1;
}

void uart_putc(unsigned char tx_byte)
{
    unsigned char next_head;

    while (1)
    {
        ES = 0;

        next_head = (tx_head + 1) & UART_BUFFER_MASK;

        if (next_head != tx_tail)
        {
            tx_buffer[tx_head] = tx_byte;
            tx_head = next_head;

            if (tx_active == 0)
            {
                tx_active = 1;
                SBUF = tx_buffer[tx_tail];
                tx_tail = (tx_tail + 1) & UART_BUFFER_MASK;
            }

            ES = 1;
            break;
        }

        ES = 1;
    }
}

void uart_puts(char code *text)
{
    while (*text != 0)
    {
        uart_putc(*text);
        text++;
    }
}

unsigned char uart_available(void)
{
    return (rx_head != rx_tail);
}

unsigned char uart_getc(void)
{
    unsigned char received_byte;

    while (rx_head == rx_tail)
    {
        ;
    }

    ES = 0;
    received_byte = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) & UART_BUFFER_MASK;
    ES = 1;

    return received_byte;
}

unsigned char uart_overflow_count(void)
{
    return rx_overflow;
}

/* Serial interrupt: RI receives bytes into the RX ring buffer.
   TI moves the next byte from the TX ring buffer into SBUF. */
void uart_isr(void) interrupt 4
{
    unsigned char received;
    unsigned char next_head;

    if (RI != 0)
    {
        received = SBUF;
        RI = 0;

        next_head = (rx_head + 1) & UART_BUFFER_MASK;

        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = received;
            rx_head = next_head;
        }
        else
        {
            rx_overflow++;
        }
    }

    if (TI != 0)
    {
        TI = 0;

        if (tx_tail != tx_head)
        {
            SBUF = tx_buffer[tx_tail];
            tx_tail = (tx_tail + 1) & UART_BUFFER_MASK;
        }
        else
        {
            tx_active = 0;
        }
    }
}
