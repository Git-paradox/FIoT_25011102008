# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 11

**DATE:** 12/10/2026

**NAME OF EXP:** HTTP Server - STM32/Wemos Wi-Fi Modem Using AT Commands

**HARDWARE:** STM32F401RE Nucleo + Wemos D1 Mini (ESP8266)

## Aim

Configure the ESP8266 as a TCP server, receive a browser HTTP request through an +IPD frame and return a static HTML HTTP response from the STM32.

## Connections

| Wemos D1 Mini | STM32 Nucleo-F401RE |
|---|---|
| TX | PA10 (USART1_RX) |
| RX | PA9 (USART1_TX) |
| GND | GND |

USART1 is configured at 115200 baud, 8-N-1. USART2 is used by the Nucleo ST-LINK Virtual COM Port at 9600 baud, 8-N-1. Connect TX to RX and use a common ground.

## Working

The STM32 sends the ESP8266 AT commands to join the local Wi-Fi network, enable multiple connections and start a TCP server on port 80:

- AT
- AT+CWMODE=1
- AT+CWJAP
- AT+CIPMUX=1
- AT+CIPSERVER=1,80

When a browser request arrives, the ESP8266 forwards it to the STM32 with an +IPD notification. The STM32 extracts the connection ID and constructs an HTTP response containing the static HTML page.

The response is sent in two stages using AT+CIPSEND: first the connection ID and exact byte length are supplied, then the HTTP response is transmitted after the ESP8266 returns the prompt.

## Configuration

Set WIFI_SSID and WIFI_PASSWORD in Core/Inc/app_config.h. The computer and Wemos must be connected to the same Wi-Fi network. The ESP8266 IP address is printed through USART2 after AT+CIFSR.

Open http://<ESP8266_IP>/ in a browser.

## Response

The HTML body contains:

- STM32 IoT Node
- Hello from STM32

The response uses HTTP/1.1 200 OK, Content-Type: text/html and the calculated Content-Length.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open MDK-ARM/TASK11A.uvprojx in Keil MDK-ARM and build the target.

## Testing

The firmware is prepared. Wi-Fi connection, browser request and response are pending hardware verification.
