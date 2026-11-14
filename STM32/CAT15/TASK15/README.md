# CAT 15 – Integrated IoT Monitoring and API Workflow

## Aim

Integrate the embedded and application-level concepts covered in the previous experiments into an end-to-end IoT monitoring system. The STM32 acquires environmental measurements from a BME280 sensor, sends the data through an ESP8266 over Wi-Fi to an MQTT broker, and makes the published values available to a Node-RED dashboard and API workflow. PuTTY is used to monitor the STM32's serial output during operation.

## System Overview

```text
BME280
  │ I2C
  ▼
STM32F401RE Nucleo
  │ UART (AT commands)
  ▼
ESP8266 / Wemos D1 mini
  │ Wi-Fi / TCP
  ▼
MQTT broker
  │ MQTT subscription
  ▼
Node-RED ─────► Dashboard
  │
  └───────────► API integration

STM32 USART2 ─► USB-to-UART / ST-LINK VCP ─► PuTTY
                (serial diagnostics)
```

The PuTTY connection is the serial-monitoring path. It displays sensor readings and connection/status messages from the STM32; the MQTT-to-Node-RED path carries the telemetry used by the dashboard/API workflow.

## Hardware

- STM32F401RE Nucleo board
- BME280 temperature, pressure and humidity sensor
- ESP8266 Wi-Fi module (Wemos D1 mini or compatible ESP8266 AT-firmware setup)
- USB connection / virtual COM port or USB-to-UART adapter for PuTTY

## Pin Connections

| Function | STM32F401RE pin | Peripheral |
|---|---|---|
| BME280 SCL | PB6 | I2C1_SCL |
| BME280 SDA | PB7 | I2C1_SDA |
| ESP8266 RX | PA9 (STM32 TX) | USART1_TX |
| ESP8266 TX | PA10 (STM32 RX) | USART1_RX |
| Serial terminal TX | PA2 | USART2_TX |
| Serial terminal RX | PA3 | USART2_RX |
| Ground | GND | Common ground |

Connect UART TX to the other device's RX, connect RX to TX, and ensure all connected modules share a common ground. The BME280 I2C lines use pull-ups as configured in the STM32 project. Confirm the voltage requirements of the ESP8266 board before wiring it to the Nucleo.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Firmware Workflow

1. The STM32 checks the BME280 at I2C address `0x76` and then `0x77`.
2. The Bosch BME280 Sensor API initializes the sensor and reads temperature, pressure and humidity.
3. The ESP8266 is initialized using AT commands over USART1 and joins the configured 2.4 GHz Wi-Fi network.
4. The firmware opens a TCP connection to the MQTT broker and sends an MQTT CONNECT packet using the custom `ESPLib` implementation.
5. The firmware publishes a JSON telemetry message to the configured topic.
6. The main loop repeats the sensor read and MQTT publish operation every five seconds.
7. USART2 prints sensor values and status messages for inspection in PuTTY.

## MQTT Configuration in the Supplied Firmware

| Setting | Value |
|---|---|
| Broker | `broker.emqx.io` |
| Port | `1883` (TCP) |
| Client ID | `STM32_BME280_Device01` |
| Publish topic | `college/weather/bme280` |
| QoS | `0` |
| Publish interval | 5 seconds |

### Published payload

```json
{
  "temperature": 25.00,
  "pressure": 1008.50,
  "humidity": 55.00
}
```

The values above illustrate the JSON structure; actual values are read from the sensor.

## Node-RED, Dashboard and API

Configure a Node-RED MQTT-in node to subscribe to:

```text
college/weather/bme280
```

Connect the MQTT input to the required parsing/function nodes, dashboard widgets and API integration nodes. The dashboard can display temperature, pressure and humidity from the JSON payload. The API side can consume or expose the parsed measurements according to the endpoint and method used in the overall project.

**Scope of this upload:** the supplied ZIP contains the STM32 firmware, Keil MDK project, BME280 driver and ESP8266/MQTT library. It does not contain a Node-RED flow export, dashboard configuration or API server source. Those application-side files should be added to this CAT 15 folder if they are maintained separately.

## PuTTY Serial Monitoring

- Select the COM port associated with the Nucleo virtual COM port or the connected USB-to-UART adapter.
- Configure the terminal for **115200 baud, 8 data bits, no parity, 1 stop bit (8-N-1)**, matching USART2 in the supplied firmware.
- Reset the board and observe the startup, sensor initialization, sensor readings and MQTT status messages.

## Project Structure

- `IOT_project/Core/` – application source, BME280 driver and ESP8266/MQTT library
- `IOT_project/Drivers/` – STM32F4 HAL and the CMSIS headers required by this target
- `IOT_project/MDK-ARM/` – Keil MDK-ARM project and startup files
- `IOT_project/IOT_project.ioc` – STM32CubeMX configuration

## Wi-Fi Configuration

Before building, edit `IOT_project/Core/Inc/app_config.h` and replace the placeholder SSID and password with the credentials for the local 2.4 GHz Wi-Fi network. Do not commit personal Wi-Fi credentials or API tokens to the repository.

## Build

1. Open `IOT_project/MDK-ARM/IOT_project.uvprojx` in Keil MDK-ARM.
2. Confirm the STM32F401RE target and required device packs are installed.
3. Update `Core/Inc/app_config.h`.
4. Build the project and program the STM32F401RE Nucleo board.
5. Open PuTTY on the correct COM port using the serial settings above.

The supplied project files are retained, but this archive has not been rebuilt in this environment. Build and hardware verification should be performed in Keil with the actual boards and network.

## Notes

- The ESP8266 must run compatible AT firmware.
- The configured public MQTT broker uses unencrypted TCP on port 1883; do not send sensitive data through this demonstration broker.
- Keep the Wi-Fi credentials local and private.
