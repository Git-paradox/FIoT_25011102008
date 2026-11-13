# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 10

**DATE:** 05/10/2026

**NAME OF EXP:** STM32-Wemos Wi-Fi Modem Interface Using AT Commands

**HARDWARE SETUP / PLATFORM:** STM32F401RE Nucleo + Wemos D1 Mini (ESP8266) + BME280 + STM32CubeMX + Keil MDK-ARM

---

## TASK 9A : STM32-Wemos UART Communication

### 1. Hardware Setup

I use two UART peripherals so the Wemos and PC terminal can communicate independently.

| Purpose | STM32 peripheral | STM32 pins |
|---|---|---|
| STM32 to Wemos | USART1 | PA9 TX, PA10 RX |
| STM32 to PC terminal | USART2 | PA2 TX, PA3 RX |
| BME280 sensor | I2C1 | PB6 SCL, PB7 SDA |

**Wemos connections:**

| Wemos D1 Mini | STM32 Nucleo-F401RE |
|---|---|
| TX | PA10 (USART1_RX) |
| RX | PA9 (USART1_TX) |
| GND | GND |
| 5V | 5V power pin, if using the D1 Mini board's 5V input |

The UART signals use 3.3V logic. I connect TX to RX, RX to TX and keep a common ground. I do not apply 5V logic to an ESP8266 UART pin.

**Serial settings:**

- Wemos UART1: 115200 baud, 8-N-1 (assumed AT firmware setting).
- PC UART2: 9600 baud, 8-N-1 through the ST-LINK Virtual COM Port.
- I2C1: 100 kHz.

The assignment does not specify baud rates. I use 115200 for the Wemos AT interface and 9600 for the PC terminal. The Wemos must have ESP8266 AT firmware installed; an Arduino sketch using ESP8266WiFi.h will not respond to AT commands.

### 2. Circuit Diagram

![Circuit Diagram](circuit.png)

### 3. Working

The STM32 sends AT followed by CR/LF through USART1. The Wemos response is received through USART1 and displayed on the PC terminal through USART2. The program also accepts AT commands entered in the PC terminal and forwards them to the Wemos.

**Pseudocode:**

1. Initialize HAL, GPIO, USART1, USART2 and I2C1.
2. Detect and initialize the BME280 sensor.
3. Send AT to the Wemos and display its response on the PC.
4. Read the firmware version and set Wi-Fi station mode.
5. Scan nearby access points.
6. Connect to the configured access point.
7. Read the Wemos IP address and connection status.
8. Read BME280 temperature, pressure and humidity values.
9. Display the sensor values on the PC terminal.
10. If a TCP server is configured, transmit the sensor data through the Wemos.

---

## TASK 9B : ESP8266 AT Command Investigation

| Requirement | AT command |
|---|---|
| Check modem response | AT |
| Check firmware/version | AT+GMR |
| Set Wi-Fi station mode | AT+CWMODE=1 |
| Scan available networks | AT+CWLAP |
| Connect to an access point | AT+CWJAP="SSID","PASSWORD" |
| Obtain IP information | AT+CIFSR |
| Query Wi-Fi state | AT+CWSTATE? |
| Check TCP connection status | AT+CIPSTATUS |
| Establish a TCP connection | AT+CIPSTART="TCP","SERVER_IP",PORT |
| Send application data | AT+CIPSEND=LENGTH |

The modem returns its response to each command. The firmware sends commands with CR/LF at the end and forwards the response to the PC terminal.

---

## TASK 9C : Connect Wemos to Wi-Fi

### 1. Wi-Fi Configuration

Before building, I replace the placeholders in Core/Inc/app_config.h:

- WIFI_SSID
- WIFI_PASSWORD
- TCP_SERVER_IP
- TCP_SERVER_PORT

The program configures station mode, scans access points, joins the selected network and queries the assigned IP address. For TCP data transmission, a TCP server must be running at the configured IP address and port.

The ESP8266 supports 2.4 GHz Wi-Fi. I use a compatible 2.4 GHz access point.

### 2. Sensor Data

The BME280 is connected through I2C1:

| BME280 | STM32 Nucleo-F401RE |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | PB6 (I2C1_SCL) |
| SDA | PB7 (I2C1_SDA) |

The firmware checks both common I2C addresses, 0x76 and 0x77, reads the calibration data and calculates:

- Temperature in degrees Celsius.
- Pressure in pascals.
- Relative humidity in percent.

A sensor data line is displayed on the PC terminal every 5 seconds. When the TCP connection is available, the same line is sent using AT+CIPSEND.

Example data format:

T=25.40C,P=100812Pa,H=56.20%

### 3. PuTTY Settings

- **Connection Type:** Serial
- **Serial Line:** ST-LINK Virtual COM Port
- **Speed:** 9600
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None
- **Local Echo:** Off

Useful PC terminal commands:

- AT - send an AT command to the Wemos.
- WIFI - repeat the Wi-Fi and TCP connection sequence.
- STATUS - query IP and TCP status.
- SENSOR - read and display the BME280 values.

### 4. Assumptions and Testing

The assignment does not specify the UART baud rates, Wi-Fi credentials, TCP server address/port or BME280 I2C address. I have used the settings listed above and included editable placeholders for the network and server details.

Hardware testing and observed terminal output will be recorded after testing the Wemos, sensor and TCP server. The circuit diagram will be added separately.

**Firmware Output:** MDK-ARM/Objects/CAT10.hex

---

## Inference

The STM32 communicates with the Wemos through USART1 and uses USART2 to display modem responses on the PC terminal. It controls the ESP8266 Wi-Fi connection with AT commands, reads environmental values from the BME280 sensor over I2C, and can send the sensor data to a TCP server.
