# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 8

**DATE:** 18/09/2026

**NAME OF EXP:** STM32 GPIO Bring-Up and Switch-Controlled LED

**HARDWARE SETUP / PLATFORM:** STM32F401RE Nucleo + STM32CubeMX + Keil MDK-ARM

---

## TASK 8A : GPIO Input/Output

### 1. Hardware Setup

The experiment uses the user push-button and onboard LED of the STM32 Nucleo board.

- **Development Board:** NUCLEO-F401RE
- **Microcontroller:** STM32F401RETx
- **LED:** LD2, PA5
- **Push-Button:** B1 USER, PC13
- **LED Active State:** HIGH
- **Button Active State:** LOW
- **GPIO Library:** STM32 HAL

**Assumptions:** I have used the onboard LD2 LED on PA5 and B1 user button on PC13. The button is configured with an internal pull-up and is treated as active-low. The project uses the internal 16 MHz HSI clock.

### 2. Circuit Diagram

![Circuit Diagram](circuit.png)

### 3. Core Logic / Pseudocode / Code Base

**Working Principle:**

The button is configured as a GPIO input and the LED as a GPIO output. The LED follows the current state of the push-button.

**Pseudocode:**

1. Initialize the HAL library.
2. Configure PA5 as LED output.
3. Configure PC13 as button input.
4. Read the button state.
5. Turn the LED ON while the button is pressed.
6. Turn the LED OFF when the button is released.
7. Repeat continuously.

**Code Base:**

The program is written in Embedded C using STM32 HAL.

- `HAL_GPIO_ReadPin()` reads the button state.
- `HAL_GPIO_WritePin()` controls the LED.
- `HAL_Init()` initializes the HAL library.

---

## TASK 8B : Event-Based Control

### 1. Core Logic / Pseudocode / Code Base

**Working Principle:**

The application toggles the LED once for every valid button press. A non-blocking software debounce check is used to prevent switch bounce from producing multiple toggles.

**Pseudocode:**

1. Read the button state.
2. Detect a change in the raw input.
3. Record the time of the change using `HAL_GetTick()`.
4. Wait for the input to remain stable for the debounce interval.
5. Toggle the LED on a confirmed press.
6. Repeat continuously.

**Code Base:**

- `HAL_GetTick()` provides the millisecond time base.
- `HAL_GPIO_ReadPin()` reads the button.
- `HAL_GPIO_TogglePin()` toggles the LED.
- A 25 ms debounce interval is used.

---

## TASK 8C : Short Press vs Long Press

### 1. Core Logic / Pseudocode / Code Base

**Working Principle:**

The application records the button press time and continues running without blocking. When the button is released, the press duration is calculated and classified.

- **Short press:** Less than 1 second - toggle the LED.
- **Long press:** 1 second or more - enter continuous LED blinking mode.
- **Next short press:** Stop blinking.
- **Long press again:** Return to blinking mode.

**Pseudocode:**

1. Detect a debounced button press.
2. Record the press start time using `HAL_GetTick()`.
3. Continue executing the application.
4. Detect a debounced button release.
5. Record the release time.
6. Calculate the press duration.
7. Classify the event as short or long press.
8. Toggle the LED or enter/stop blinking mode.
9. Repeat continuously.

**Code Base:**

The final application is written in Embedded C using STM32 HAL.

- `HAL_GetTick()` measures press duration.
- `HAL_GPIO_ReadPin()` reads the button state.
- `HAL_GPIO_TogglePin()` toggles the LED.
- The 25 ms debounce check handles switch bounce.
- The 250 ms interval controls blinking.
- No blocking delay is used for long-press detection.

### 4. Inference

The STM32 reads the button state and controls the onboard LED. Valid short presses toggle the LED, while long presses activate continuous blinking mode. The application uses a non-blocking time-based method to classify button presses.

The experiment demonstrates GPIO input/output, event-based switch handling, software debouncing and short/long-press detection using STM32 HAL.

### Build Output

- **Keil Project:** `MDK-ARM/CAT8.uvprojx`
- **Firmware Output:** `MDK-ARM/Objects/CAT8.hex`
