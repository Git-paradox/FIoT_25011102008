# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 10

**DATE:** 05/10/2026

**NAME OF EXP:** STM32-Wemos UART Communication

**HARDWARE:** STM32F401RE Nucleo + Wemos D1 Mini (ESP8266)

## Aim

Establish UART communication between the STM32 and Wemos D1 Mini and display the Wemos response on the PC terminal.

## UART Configuration

| Purpose | Peripheral | STM32 pins | Settings |
|---|---|---|---|
| STM32 to Wemos | USART1 | PA9 TX, PA10 RX | 115200, 8-N-1 |
| STM32 to PC | USART2 | PA2 TX, PA3 RX | 9600, 8-N-1 |

The Wemos baud rate is an assumption and must match the installed ESP8266 AT firmware.

## Wemos Connections

| Wemos pin | STM32 pin |
|---|---|
| TX | PA10 (USART1_RX) |
| RX | PA9 (USART1_TX) |
| GND | GND |

Connect TX to RX, RX to TX and use a common ground. UART signal logic is 3.3V. Power the Wemos board using its appropriate power input.

## Working

After initializing both UART peripherals, the STM32 sends \`AT\` followed by CR/LF to the Wemos. The response is forwarded to the PC terminal. Characters can also be passed between the terminal and Wemos.

The Wemos must have ESP8266 AT firmware installed. An ordinary Arduino Wi-Fi sketch does not respond to AT commands.

## Algorithm

1. Initialize HAL, USART1 and USART2.
2. Send \`AT\r\n\` to the Wemos through USART1.
3. Receive the Wemos response through USART1.
4. Forward received characters to the PC through USART2.
5. Continue passing UART characters between the Wemos and PC.

## PuTTY Settings

- Serial line: ST-LINK Virtual COM Port.
- Speed: 9600.
- Data bits: 8.
- Stop bits: 1.
- Parity: None.
- Flow control: None.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open \`MDK-ARM/TASK10A.uvprojx\` in Keil MDK-ARM and build the target. The HEX output is created in \`MDK-ARM/Objects/\`.

## Testing

Hardware testing of the standalone Task 10A project is pending.
