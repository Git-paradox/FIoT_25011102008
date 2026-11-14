# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 12

**DATE:** 14/10/2026

**NAME OF EXP:** Short-Press and Long-Press Detection

**HARDWARE:** STM32F401RE Nucleo

## Aim

Extend the button-controlled LED application to distinguish between a short button press and a long button press.

## Configuration

- Board: NUCLEO-F401RE.
- LED: LD2 on PA5, active high.
- Button: B1 on PC13, active low.
- Button input: internal pull-up.
- Software debounce: 25 ms.
- Long-press threshold in this implementation: 1000 ms.
- Blink interval in this implementation: 250 ms.

## Working

The application records the time when a debounced button press begins. On release, it checks the press duration. A short press toggles the LED state. A long press enables or disables the LED blinking mode.

The assignment requires different actions for short and long presses but does not prescribe the exact actions. The short-toggle/long-blink behavior is the implementation used here.

## Algorithm

1. Initialize HAL and GPIO.
2. Debounce the PC13 button input.
3. Record the tick when a stable press begins.
4. On release, calculate the press duration.
5. For a short press, toggle the LED state.
6. For a long press, toggle blinking mode.
7. While blinking mode is active, toggle LD2 at the configured interval.

## Circuit Diagram

![Circuit Diagram](circuit.svg)

## Build

Open MDK-ARM/TASK12C.uvprojx in Keil MDK-ARM and build the target.

## Testing

The standalone Task12C project is prepared. Hardware testing is pending.
