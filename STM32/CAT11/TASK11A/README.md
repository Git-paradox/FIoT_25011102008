# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 11

**DATE:** 14/10/2026

**NAME OF EXP:** On-board LED Bring-Up

**HARDWARE:** STM32F401RE Nucleo

## Aim

Develop and run a program to verify the NUCLEO-F401RE GPIO output by blinking the on-board LED at a fixed interval.

## Configuration

- Board: NUCLEO-F401RE.
- LED: LD2 on PA5, active high.
- Clock: internal HSI, 16 MHz.
- Blink interval: 500 ms ON/OFF interval used for this implementation.

## Working

The program configures PA5 as a GPIO output and toggles LD2 every 500 ms.

## Algorithm

1. Initialize HAL and the system clock.
2. Enable GPIOA clock.
3. Configure PA5 as push-pull output.
4. Toggle PA5.
5. Wait 500 ms and repeat.

## Code

The program uses HAL_GPIO_TogglePin() and HAL_Delay().

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open MDK-ARM/TASK11A.uvprojx in Keil MDK-ARM and build the target. The HEX output is created in MDK-ARM/Objects/.

## Testing

The standalone Task11A project is prepared. Hardware testing is pending.
