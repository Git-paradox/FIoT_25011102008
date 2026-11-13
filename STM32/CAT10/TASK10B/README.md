# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 10

**DATE:** 05/10/2026

**NAME OF EXP:** ESP8266 AT Command Interface

**HARDWARE:** STM32F401RE Nucleo + Wemos D1 Mini (ESP8266) + PC terminal

## Aim

Explore the ESP8266 AT command interface by sending commands from a PC terminal through the STM32 and displaying the Wemos responses.

## UART Configuration

| Purpose | Peripheral | STM32 pins | Settings |
|---|---|---|---|
| STM32 to Wemos | USART1 | PA9 TX, PA10 RX | 115200, 8-N-1 |
| STM32 to PC | USART2 | PA2 TX, PA3 RX | 9600, 8-N-1 |

The Wemos UART baud rate is an assumption and must match its AT firmware.

## Working

A command entered in the PC terminal is collected until CR or LF is received. The STM32 forwards the command to the Wemos with CR/LF and displays the modem response.

## AT Command Investigation

| Requirement | AT command |
|---|---|
| Check modem response | \`AT\` |
| Check firmware/version | \`AT+GMR\` |
| Set Wi-Fi station mode | \`AT+CWMODE=1\` |
| Scan available networks | \`AT+CWLAP\` |
| Connect to an access point | \`AT+CWJAP="SSID","PASSWORD"\` |
| Obtain IP information | \`AT+CIFSR\` |
| Check TCP status | \`AT+CIPSTATUS\` |
| Establish TCP connection | \`AT+CIPSTART="TCP","SERVER_IP",PORT\` |
| Send application data | \`AT+CIPSEND=LENGTH\` |

The table records the command names to investigate for the listed modem functions.

## Algorithm

1. Initialize HAL, USART1 and USART2.
2. Receive a command line from the PC terminal.
3. Echo the entered characters.
4. Send the completed command to the Wemos with CR/LF.
5. Receive the Wemos response.
6. Display the response on the PC terminal.
7. Repeat.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open \`MDK-ARM/TASK10B.uvprojx\` in Keil MDK-ARM and build the target. The HEX output is created in \`MDK-ARM/Objects/\`.

## Testing

Hardware testing of the standalone Task 10B project is pending.
