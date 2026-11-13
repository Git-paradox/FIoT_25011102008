# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION 12

**DATE:** 23/10/2026

**NAME OF EXP:** BME280 Sensor Interfacing Using I2C and Bosch Sensor API

**HARDWARE:** STM32F401RE Nucleo + BME280

## Aim

Interface the BME280 sensor with the STM32 using I2C and use the official Bosch SensorAPI to obtain compensated temperature, pressure and humidity readings.

## Driver Files

This project includes the three Bosch SensorAPI files:

- bme280.c
- bme280.h
- bme280_defs.h

The Bosch driver performs the compensation calculations. The STM32 application provides the I2C read, I2C write and delay callback functions.

## Connections

| BME280 | STM32 Nucleo-F401RE |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | PB6 (I2C1_SCL) |
| SDA | PB7 (I2C1_SDA) |

I2C1 is configured at 100 kHz. The primary BME280 address is 0x76. Use the secondary address 0x77 if the SDO pin is high.

## Sensor Configuration

- Temperature oversampling: 1x.
- Pressure oversampling: 1x.
- Humidity oversampling: 1x.
- Filter: disabled.
- Sensor mode: normal.
- Standby time: 1000 ms.

The Bosch API is configured using bme280_init(), bme280_set_sensor_settings(), bme280_set_sensor_mode() and bme280_get_sensor_data().

## Output

The STM32 reads the compensated values once per second and displays temperature in °C, pressure in hPa and humidity in %RH through USART2.

PuTTY settings: 9600 baud, 8 data bits, no parity, 1 stop bit, no flow control.

## Circuit Diagram

![Circuit Diagram](circuit.png)

## Build

Open MDK-ARM/TASK12A.uvprojx in Keil MDK-ARM and build the target. The project includes the official Bosch SensorAPI source and headers.

## Testing

The project and driver integration are prepared. Sensor communication and live readings are pending hardware verification.
