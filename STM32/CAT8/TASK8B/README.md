# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 8

**DATE:** 18/09/2026

**NAME OF EXP:** Event-Based Switch Control with Debouncing

**HARDWARE:** STM32F401RE Nucleo

## Aim

Toggle the onboard LED once for each valid push-button press and avoid repeated toggles caused by switch bounce.

## Pin Configuration

- LD2 LED: PA5, output, active high.
- B1 user button: PC13, input, active low, internal pull-up.
- Clock: internal 16 MHz HSI.

## Working

The firmware polls the button and checks that its state remains stable for 25 ms. A stable transition to the pressed state toggles the LED once. The loop is non-blocking and uses \`HAL_GetTick()\` for the debounce interval.

## Algorithm

1. Initialize HAL, LED and button GPIO.
2. Read the raw button state and current tick.
3. When the raw state changes, record the change time.
4. After 25 ms of stability, update the debounced state.
5. Toggle the LED only on a debounced press.
6. Repeat.

## Code

The program uses \`HAL_GPIO_ReadPin()\`, \`HAL_GPIO_TogglePin()\` and \`HAL_GetTick()\`.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open \`MDK-ARM/TASK8B.uvprojx\` in Keil MDK-ARM and build the target. The HEX output is created in \`MDK-ARM/Objects/\`.

## Testing

The project files are separated for Task 8B. Physical testing of this standalone task build is pending.
