#include <REGX51.H>

sbit ROW1 = P1^0;
sbit ROW2 = P1^1;
sbit ROW3 = P1^2;
sbit ROW4 = P1^3;

sbit COL1 = P1^4;
sbit COL2 = P1^5;
sbit COL3 = P1^6;
sbit COL4 = P1^7;

unsigned char current_mode = 0;
unsigned char key_value = 0xFF;
unsigned char key_ready = 0;

unsigned char pattern_tick = 0;
unsigned char running_index = 0;
unsigned char binary_value = 0;
unsigned char alternate_state = 0;
unsigned char blink_state = 1;

unsigned char candidate_key = 0xFF;
unsigned char key_stable_count = 0;
unsigned char key_latched = 0;

unsigned char code row_mask[4] = {0xFE, 0xFD, 0xFB, 0xF7};

unsigned char code key_map[4][4] = {
    {1, 2, 3, 0xFF},
    {4, 5, 6, 0xFF},
    {7, 8, 9, 0xFF},
    {0xFF, 0, 0xFF, 0xFF}
};

/* Common-cathode seven-segment codes: P2.0=a through P2.7=dp. */
unsigned char code segment_code[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

void delay_ms(unsigned int milliseconds)
{
    volatile unsigned int i;
    volatile unsigned char j;

    for (i = 0; i < milliseconds; i++)
    {
        for (j = 0; j < 200; j++)
        {
            ;
        }
    }
}

void set_leds(unsigned char pattern)
{
    P3 = (P3 & 0x0F) | (((~pattern) & 0x0F) << 4);
}

unsigned char read_keypad(void)
{
    unsigned char row;
    unsigned char columns;
    unsigned char col;

    for (row = 0; row < 4; row++)
    {
        P1 = row_mask[row];
        columns = P1 & 0xF0;

        if (columns != 0xF0)
        {
            if ((columns & 0x10) == 0)
                col = 0;
            else if ((columns & 0x20) == 0)
                col = 1;
            else if ((columns & 0x40) == 0)
                col = 2;
            else
                col = 3;

            P1 = 0xFF;
            return key_map[row][col];
        }
    }

    P1 = 0xFF;
    return 0xFF;
}

void keypad_scan(void)
{
    unsigned char raw_key;

    raw_key = read_keypad();

    if (raw_key == 0xFF)
    {
        candidate_key = 0xFF;
        key_stable_count = 0;
        key_latched = 0;
    }
    else if (raw_key != candidate_key)
    {
        candidate_key = raw_key;
        key_stable_count = 0;
    }
    else
    {
        if (key_stable_count < 2)
            key_stable_count++;

        if ((key_stable_count >= 2) && (key_latched == 0))
        {
            key_value = raw_key;
            key_ready = 1;
            key_latched = 1;
        }
    }
}

void display_digit(unsigned char value)
{
    if (value <= 9)
        P2 = segment_code[value];
    else
        P2 = 0x00;
}

void running_led(void)
{
    set_leds((unsigned char)(1 << running_index));
    running_index++;
    running_index &= 0x03;
}

void binary_counter(void)
{
    set_leds(binary_value);
    binary_value++;
    binary_value &= 0x0F;
}

void alternate_led(void)
{
    if (alternate_state == 0)
    {
        set_leds(0x0A);
        alternate_state = 1;
    }
    else
    {
        set_leds(0x05);
        alternate_state = 0;
    }
}

void blink_all(void)
{
    if (blink_state != 0)
    {
        set_leds(0x0F);
        blink_state = 0;
    }
    else
    {
        set_leds(0x00);
        blink_state = 1;
    }
}

void led_control(unsigned char key)
{
    current_mode = key;
    pattern_tick = 0;
    running_index = 0;
    binary_value = 0;
    alternate_state = 0;
    blink_state = 1;

    switch (key)
    {
        case 1:
            set_leds(0x01);
            break;

        case 2:
            set_leds(0x02);
            break;

        case 3:
            set_leds(0x04);
            break;

        case 4:
            set_leds(0x08);
            break;

        case 5:
            running_led();
            break;

        case 6:
            alternate_led();
            break;

        case 7:
            binary_counter();
            break;

        case 8:
            blink_all();
            break;

        case 9:
        case 0:
            set_leds(0x00);
            break;

        default:
            current_mode = 0;
            set_leds(0x00);
            break;
    }
}

void main(void)
{
    P1 = 0xFF;
    P2 = 0x00;
    set_leds(0x00);

    while (1)
    {
        keypad_scan();

        if (key_ready != 0)
        {
            key_ready = 0;
            display_digit(key_value);
            led_control(key_value);
        }

        if (pattern_tick >= 20)
        {
            pattern_tick = 0;

            if (current_mode == 5)
                running_led();
            else if (current_mode == 6)
                alternate_led();
            else if (current_mode == 7)
                binary_counter();
            else if (current_mode == 8)
                blink_all();
        }
        else
        {
            pattern_tick++;
        }

        delay_ms(10);
    }
}
