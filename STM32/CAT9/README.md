# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 9

**DATE:** 28/09/2026

**NAME OF EXP:** STM32 UART Command Controlled LED

**HARDWARE SETUP / PLATFORM:** STM32F401RE Nucleo + STM32CubeMX + Keil MDK-ARM

---

## TASK 9A : Basic UART Communication

### 1. Hardware Setup

The experiment uses UART communication between the STM32 Nucleo board and a PC terminal.

- **Development Board:** NUCLEO-F401RE
- **Microcontroller:** STM32F401RETx
- **UART Peripheral:** USART2
- **TX Pin:** PA2
- **RX Pin:** PA3
- **Baud Rate:** 9600
- **Frame Format:** 8-N-1
- **Interface:** ST-LINK Virtual COM Port
- **Terminal Software:** PuTTY
- **LED:** LD2, PA5

The question does not specify the UART baud rate, so I have used 9600 baud, 8 data bits, no parity and 1 stop bit. I have used USART2 on PA2/PA3 through the onboard ST-LINK Virtual COM Port. The LED is the onboard LD2 on PA5.

**Firmware Output:** The compiled file will be saved at `MDK-ARM/Objects/CAT9.hex`.

### 2. Circuit Diagram

![Circuit Diagram](circuit.png)

### 3. Core Logic / Pseudocode / Code Base

**Working Principle:**

USART2 is initialized using STM32 HAL. The program transmits the character 'A' repeatedly to the PC terminal. Every character received from the PC is transmitted back to the terminal.

**Pseudocode:**

1. Initialize the HAL library.
2. Configure USART2 for 9600-8-N-1 communication.
3. Transmit the character 'A' once every second.
4. Receive one character from the PC terminal.
5. Transmit the received character back to the PC.
6. Repeat continuously.

**Code Base:**

The program is written in Embedded C using STM32 HAL.

- `HAL_UART_Init()` initializes the UART peripheral.
- `HAL_UART_Transmit()` transmits a character or string.
- `HAL_UART_Receive()` receives a character from the PC terminal.

---

## TASK 9B : String-Based Application Logic

### 1. Core Logic / Pseudocode / Code Base

**Working Principle:**

The program receives a command string from the terminal, echoes the received characters and checks the command after a line ending is received.

- **LED ON:** Turns the onboard LED ON.
- **LED OFF:** Turns the onboard LED OFF.
- **LED TOGGLE:** Changes the current LED state.

Commands are accepted in uppercase or lowercase.

**Pseudocode:**

1. Receive characters from the PC terminal.
2. Echo each received character.
3. Store the characters in a command buffer.
4. Detect carriage return or line feed.
5. Compare the completed command with the supported commands.
6. Control the onboard LED.
7. Send the command result to the terminal.
8. Clear the buffer and continue.

**Code Base:**

- `HAL_UART_Receive()` reads terminal input.
- `HAL_UART_Transmit()` echoes characters and sends responses.
- `HAL_GPIO_WritePin()` turns the LED ON or OFF.
- `HAL_GPIO_TogglePin()` toggles the LED.
- `HAL_GetTick()` provides the periodic transmission time base.

### 2. PuTTY Settings

- **Connection Type:** Serial
- **Serial Line:** ST-LINK Virtual COM Port
- **Speed:** 9600
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None
- **Local Echo:** Off

### 3. CR and LF Tests

The program accepts either carriage return (`\r`) or line feed (`\n`) as a command terminator. When PuTTY sends CRLF, the command is processed once at the CR and the following LF is ignored as an empty line.

The following test strings can be entered and observed in PuTTY:

- `HELLO\r`
- `HELLO\n`
- `HELLO\r\n`
- `HELLO\r\nWORLD\r\n`

Record the terminal behaviour and explain the difference between carriage return and line feed.

### 4. Inference

The STM32 communicates with the PC terminal through USART2. It repeatedly transmits a character, receives and echoes terminal input, and processes string commands to control the onboard LED.

The experiment demonstrates UART initialization, character transmission and reception, string-based command processing, GPIO control and the use of carriage return and line feed in serial communication.
