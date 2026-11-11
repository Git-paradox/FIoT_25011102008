#include <REGX51.H>

/* Four external LEDs connected to P1.0-P1.3.
   Active-low wiring: LOW turns an LED ON. */
sbit LED1 = P1^0;
sbit LED2 = P1^1;
sbit LED3 = P1^2;
sbit LED4 = P1^3;

/* Push-button connected to INT0 on P3.2, active LOW. */
sbit BUTTON = P3^2;

volatile unsigned char tick_10ms = 0;
volatile unsigned char button_irq_flag = 0;

unsigned char current_mode = 0;
unsigned char mode_last_tick = 0;
unsigned char running_index = 0;
unsigned char binary_count = 0;
unsigned char alternating_state = 0;
unsigned char flash_state = 1;

void set_led_pattern(unsigned char pattern)
{
    LED1 = (pattern & 0x01) ? 0 : 1;
    LED2 = (pattern & 0x02) ? 0 : 1;
    LED3 = (pattern & 0x04) ? 0 : 1;
    LED4 = (pattern & 0x08) ? 0 : 1;
}

void start_current_mode(void)
{
    mode_last_tick = tick_10ms;

    if (current_mode == 1)
    {
        running_index = 0;
        set_led_pattern(0x01);
    }
    else if (current_mode == 2)
    {
        binary_count = 0;
        set_led_pattern(0x00);
    }
    else if (current_mode == 3)
    {
        alternating_state = 0;
        set_led_pattern(0x0A);
    }
    else if (current_mode == 4)
    {
        flash_state = 1;
        set_led_pattern(0x0F);
    }
}

void next_mode(void)
{
    if (current_mode >= 4)
        current_mode = 1;
    else
        current_mode++;

    start_current_mode();
}

void update_mode(unsigned char now)
{
    if (current_mode == 0)
        return;

    if ((unsigned char)(now - mode_last_tick) < 50)
        return;

    mode_last_tick = now;

    if (current_mode == 1)
    {
        running_index++;
        running_index &= 0x03;
        set_led_pattern((unsigned char)(1 << running_index));
    }
    else if (current_mode == 2)
    {
        binary_count++;
        if (binary_count >= 16)
            binary_count = 0;

        set_led_pattern(binary_count);
    }
    else if (current_mode == 3)
    {
        alternating_state = !alternating_state;

        if (alternating_state == 0)
            set_led_pattern(0x0A);
        else
            set_led_pattern(0x05);
    }
    else if (current_mode == 4)
    {
        flash_state = !flash_state;

        if (flash_state != 0)
            set_led_pattern(0x0F);
        else
            set_led_pattern(0x00);
    }
}

/* Timer 0 creates a 10 ms system tick.
   Reload 0xDC00 assumes an 11.0592 MHz, 12-clock 8051. */
void timer0_isr(void) interrupt 1
{
    TH0 = 0xDC;
    TL0 = 0x00;
    tick_10ms++;
}

/* INT0 detects the button press. Debouncing is handled in main(). */
void external0_isr(void) interrupt 0
{
    EX0 = 0;
    button_irq_flag = 1;
}

void main(void)
{
    unsigned char now;
    unsigned char debounce_state = 0;
    unsigned char debounce_start = 0;

    BUTTON = 1;
    set_led_pattern(0x00);

    TMOD = (TMOD & 0xF0) | 0x01;
    TH0 = 0xDC;
    TL0 = 0x00;

    IT0 = 1;
    EX0 = 0;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
    EX0 = 1;

    while (1)
    {
        now = tick_10ms;

        if (button_irq_flag != 0)
        {
            button_irq_flag = 0;
            debounce_state = 1;
            debounce_start = now;
        }

        if (debounce_state == 1)
        {
            if ((unsigned char)(now - debounce_start) >= 2)
            {
                if (BUTTON == 0)
                {
                    next_mode();
                    debounce_state = 2;
                }
                else
                {
                    debounce_state = 0;
                    EX0 = 1;
                }
            }
        }
        else if (debounce_state == 2)
        {
            if (BUTTON == 1)
            {
                debounce_state = 3;
                debounce_start = now;
            }
        }
        else if (debounce_state == 3)
        {
            if ((unsigned char)(now - debounce_start) >= 2)
            {
                if (BUTTON == 1)
                {
                    debounce_state = 0;
                    EX0 = 1;
                }
                else
                {
                    debounce_state = 2;
                }
            }
        }

        update_mode(now);
    }
}
