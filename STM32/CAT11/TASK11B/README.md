# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 11

**DATE:** 12/10/2026

**NAME OF EXP:** Node-RED as a Second HTTP Client of the STM32/Wemos HTTP Server

**HARDWARE:** STM32F401RE Nucleo + Wemos D1 Mini (ESP8266) + Node-RED PC

## Aim

Use Node-RED as a second HTTP client to request the page served by the STM32/Wemos HTTP server and display the returned HTTP response in the Debug sidebar.

## Firmware

The STM32 and ESP8266 use the same TCP server implementation as TASK11A:

- ESP8266 station mode and Wi-Fi connection.
- Multiple TCP connections using AT+CIPMUX=1.
- HTTP server on port 80 using AT+CIPSERVER=1,80.
- Incoming browser/Node-RED requests are received through +IPD.
- HTTP responses are sent using AT+CIPSEND with the connection ID and response length.

## Node-RED Flow

Import Tools/NodeRED_flow.json into Node-RED and deploy it. The flow is:

**Inject → HTTP Request → Debug**

Set the HTTP Request node URL to http://<ESP8266_IP>/. The Inject node triggers the request and the Debug node displays the HTML response received from the STM32.

## Configuration

Set WIFI_SSID and WIFI_PASSWORD in Core/Inc/app_config.h. Replace the sample IP address in the Node-RED HTTP Request node with the IP shown by the ESP8266 AT+CIFSR command.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open MDK-ARM/TASK11B.uvprojx in Keil MDK-ARM and build the target.

## Testing

The firmware and Node-RED flow are prepared. The Node-RED request and Debug output are pending hardware/network verification.
