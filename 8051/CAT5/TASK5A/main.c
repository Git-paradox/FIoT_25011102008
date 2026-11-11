#include <REGX51.H>

/* Output assignments used for this experiment. */
sbit LED_1SEC = P1^0;       /* Active-low LED: toggles once per second. */
sbit LED_PWM = P1^1;        /* Active-low LED: 500 Hz software PWM. */
sbit SQUARE_OUT = P1^2;     /* Variable-frequency square-wave output. */
sbit WAVE_OUT = P1^3;       /* Selectable waveform output. */

/* Set to 1 only after connecting a suitable stepper motor driver. */
#define STEPPER_ENABLE 0

#if STEPPER_ENABLE
#define STEPPER_DIRECTION 1
#define STEPPER_STEP_INTERVAL_MS 100

unsigned char code step_sequence[4] = {0x09, 0x0C, 0x06, 0x03};
unsigned char step_index = 0;
#endif

/* Timer 0: 1 ms time base, reload FC66 at 11.0592 MHz / 12. */
volatile unsigned int system_ms = 0;

/* Timer 1: approximately 100 us tick, reload FFA4. */
volatile unsigned char pwm_phase = 0;
volatile unsigned char pwm_on_ticks = 0;

volatile unsigned char square_mode = 0;
volatile unsigned char square_countdown = 50;
volatile unsigned char square_fraction = 0;

volatile unsigned char waveform_mode = 0;
volatile unsigned char wave_phase = 0;
volatile unsigned char wave_divider = 0;
volatile unsigned int burst_elapsed = 0;
volatile unsigned char burst_subtick = 0;

unsigned char duty_percent = 0;
unsigned char fade_up = 1;

unsigned char code square_half_ticks[5] = {50, 10, 5, 0, 1};

void timer0_isr(void) interrupt 1
{
    TH0 = 0xFC;
    TL0 = 0x66;
    system_ms++;
}

void update_square_wave(void)
{
    if (square_countdown > 1)
    {
        square_countdown--;
    }
    else
    {
        SQUARE_OUT = !SQUARE_OUT;

        if (square_mode == 3)
        {
            if (square_fraction == 0)
            {
                square_countdown = 2;
                square_fraction = 1;
            }
            else
            {
                square_countdown = 3;
                square_fraction = 0;
            }
        }
        else
        {
            square_countdown = square_half_ticks[square_mode];
        }
    }
}

void update_waveform(void)
{
    if (waveform_mode == 0)
    {
        wave_divider++;
        if (wave_divider >= 5)
        {
            wave_divider = 0;
            WAVE_OUT = !WAVE_OUT;
        }
    }
    else if (waveform_mode == 1)
    {
        wave_phase++;
        if (wave_phase >= 20)
            wave_phase = 0;

        WAVE_OUT = (wave_phase < 5) ? 1 : 0;
    }
    else if (waveform_mode == 2)
    {
        wave_phase++;
        if (wave_phase >= 20)
            wave_phase = 0;

        WAVE_OUT = (wave_phase == 0) ? 1 : 0;
    }
    else
    {
        if (burst_elapsed < 100)
        {
            burst_subtick++;
            if (burst_subtick >= 10)
            {
                burst_subtick = 0;
                WAVE_OUT = 1;
            }
            else
            {
                WAVE_OUT = 0;
            }
        }
        else
        {
            WAVE_OUT = 0;
        }

        burst_elapsed++;
        if (burst_elapsed >= 1100)
        {
            burst_elapsed = 0;
            burst_subtick = 0;
        }
    }
}

void timer1_isr(void) interrupt 3
{
    TH1 = 0xFF;
    TL1 = 0xA4;

    pwm_phase++;
    if (pwm_phase >= 20)
        pwm_phase = 0;

    LED_PWM = (pwm_phase < pwm_on_ticks) ? 0 : 1;

    update_square_wave();
    update_waveform();
}

void update_led_fade(void)
{
    if (fade_up != 0)
    {
        if (duty_percent < 100)
            duty_percent = duty_percent + 5;
        else
        {
            fade_up = 0;
            duty_percent = 95;
        }
    }
    else
    {
        if (duty_percent > 0)
            duty_percent = duty_percent - 5;
        else
        {
            fade_up = 1;
            duty_percent = 5;
        }
    }

    pwm_on_ticks = duty_percent / 5;
}

void update_output_modes(void)
{
    square_mode++;
    if (square_mode >= 5)
        square_mode = 0;

    square_fraction = 0;
    square_countdown = square_half_ticks[square_mode];

    waveform_mode++;
    if (waveform_mode >= 4)
        waveform_mode = 0;

    wave_phase = 0;
    wave_divider = 0;
    burst_elapsed = 0;
    burst_subtick = 0;
    WAVE_OUT = 0;
}

#if STEPPER_ENABLE
void update_stepper(void)
{
    if (STEPPER_DIRECTION != 0)
    {
        step_index++;
        step_index &= 0x03;
    }
    else
    {
        if (step_index == 0)
            step_index = 3;
        else
            step_index--;
    }

    P2 = (P2 & 0xF0) | step_sequence[step_index];
}
#endif

void main(void)
{
    unsigned int now;
    unsigned int last_led_toggle = 0;
    unsigned int last_fade_update = 0;
    unsigned int last_mode_update = 0;
#if STEPPER_ENABLE
    unsigned int last_step_update = 0;
#endif

    LED_1SEC = 1;
    LED_PWM = 1;
    SQUARE_OUT = 0;
    WAVE_OUT = 0;
    P2 = 0x00;

    TMOD = 0x11;

    TH0 = 0xFC;
    TL0 = 0x66;

    TH1 = 0xFF;
    TL1 = 0xA4;

    ET0 = 1;
    ET1 = 1;
    EA = 1;

    TR0 = 1;
    TR1 = 1;

    while (1)
    {
        ET0 = 0;
        now = system_ms;
        ET0 = 1;

        if ((unsigned int)(now - last_led_toggle) >= 1000)
        {
            last_led_toggle = last_led_toggle + 1000;
            LED_1SEC = !LED_1SEC;
        }

        if ((unsigned int)(now - last_fade_update) >= 50)
        {
            last_fade_update = last_fade_update + 50;
            update_led_fade();
        }

        if ((unsigned int)(now - last_mode_update) >= 2000)
        {
            last_mode_update = last_mode_update + 2000;
            update_output_modes();
        }

#if STEPPER_ENABLE
        if ((unsigned int)(now - last_step_update) >= STEPPER_STEP_INTERVAL_MS)
        {
            last_step_update = last_step_update + STEPPER_STEP_INTERVAL_MS;
            update_stepper();
        }
#endif
    }
}
