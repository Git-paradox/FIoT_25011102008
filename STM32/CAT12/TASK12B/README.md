# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 12

**DATE:** 14/10/2026

**NAME OF EXP:** On-board Switch Control with State Remembrance

**HARDWARE:** STM32F401RE Nucleo

## Aim

Modify the application so the on-board push button controls the LED. The LED shall toggle its state when a valid button press is detected and shall remember its previous state after the button is released.

## Configuration

- Board: NUCLEO-F401RE.
- LED: LD2 on PA5, active high.
- Button: B1 on PC13, active low.
- Button input: internal pull-up.
- Software debounce: 25 ms.
- Clock: internal HSI, 16 MHz.

## Working

The program reads PC13 and applies a 25 ms debounce interval. On each confirmed button press, it toggles the stored LED state and writes that state to PA5. The LED therefore remains in its selected state after the button is released.

## Algorithm

1. Initialize HAL and GPIO.
2. Configure PA5 as output and PC13 as input with pull-up.
3. Read the button input continuously.
4. Accept a state change only after the debounce interval.
5. Toggle the stored LED state on a confirmed press.
6. Repeat.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open MDK-ARM/TASK12B.uvprojx in Keil MDK-ARM and build the target.

## Testing

The standalone Task12B project is prepared. Hardware testing is pending.
