# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 3

**DATE:** 27/07/2026

**NAME OF EXP:** Timer-Based LED Pattern Generator and Button Interrupt Controller

**HARDWARE SETUP / PLATFORM:** AT89S52 Microcontroller + Keil µVision

---

## TASK 3A : Timer-Based LED Pattern Generator and Button Interrupt Controller

### 1. Hardware Setup

The experiment uses four LEDs as digital outputs and a push-button as a digital input to change the LED operating mode.

- **Microcontroller:** AT89S52
- **CPU Core:** 8-bit 8051
- **LED GPIO Port:** Port 1
- **LED GPIO Pins:** P1.0, P1.1, P1.2 and P1.3
- **Button Pin:** P3.2 / INT0
- **Peripheral:** Four external LEDs and one push-button
- **Power Supply:** 5V DC
- **LED Active State:** LOW
- **Crystal Frequency:** 11.0592 MHz
- **Timer:** Timer 0, 10 ms tick
- **Button Debounce:** 20 ms

**Assumptions:** I have used P1.0-P1.3 for the four LEDs and P3.2/INT0 for the push-button. The LEDs are assumed to be active-low with individual 330 Ω current-limiting resistors. The button is assumed to connect P3.2 to GND when pressed, using the Port 3 internal pull-up. The clock is assumed to be 11.0592 MHz.

### 2. Circuit Diagram

![Circuit Diagram](circuit.png)

### 3. Core Logic / Pseudocode / Code Base

**Working Principle:**

The four LEDs are controlled directly through Port 1. The push-button is connected to external interrupt INT0. Each valid button press changes the system to the next operating mode.

Timer 0 generates a 10 ms tick, which is used for LED timing and button debouncing. A 20 ms debounce check ensures that one physical press produces only one mode change.

**Operating Modes:**

- **Mode 1 — Running LED:** LEDs turn ON one by one from LED1 to LED4 and repeat.
- **Mode 2 — Binary Counter:** LEDs display the binary count from 0000 to 1111 and repeat.
- **Mode 3 — Alternating Pattern:** LEDs alternate between 1010 and 0101.
- **Mode 4 — All LED Flash:** All LEDs turn ON and OFF together.

The system starts with all LEDs OFF. Each valid button press advances to the next mode. After Mode 4, the next press returns to Mode 1.

**Pseudocode:**

1. Configure four LED pins as outputs and the button on INT0 as input.
2. Configure Timer 0 to generate a 10 ms tick.
3. Enable the external button interrupt.
4. Keep all LEDs OFF initially.
5. Detect a button press using INT0.
6. Debounce the button and confirm the press.
7. Advance to the next LED operating mode.
8. Update the LED pattern every 500 ms using the timer tick.
9. Repeat continuously.

**Code Base:**

The program is written in Embedded C using Keil C51.

- `REGX51.H` provides the 8051 register definitions.
- `P1.0-P1.3` control the four LEDs.
- `P3.2 / INT0` is used for the push-button interrupt.
- `timer0_isr()` generates the 10 ms timer tick.
- `external0_isr()` detects the button press.
- `set_led_pattern()` controls the LED outputs.
- `update_mode()` generates the four LED patterns.
- A 20 ms debounce check prevents repeated mode changes from one button press.

### 4. Inference

The system changes between four LED operating modes whenever the push-button is pressed. Timer 0 controls the LED pattern timing, while the external interrupt detects button presses.

The experiment demonstrates direct GPIO register access, timer interrupt programming, external interrupt handling and switch debouncing using bare-metal Embedded C.
