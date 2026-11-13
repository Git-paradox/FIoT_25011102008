# Foundations of IoT — Lab Submissions

Repository for recording Foundations of IoT continuous assessment tasks.

## Platforms

- `8051/` — 8051 microcontroller tasks
- `STM32/` — STM32 microcontroller tasks

Each submission is stored in its own task folder. Every STM32 task has a separate `.ioc`, `.mxproject`, `Core/`, `Drivers/`, `MDK-ARM/` project, startup file and `README.md`. Circuit diagrams are kept as `circuit.png` in the corresponding task folder.

## Current submissions

| Platform | CAT | Task | Description |
|---|---|---|---|
| 8051 | CAT2 | TASK2B | Bare-metal GPIO LED blink with custom delay function |
| 8051 | CAT3 | TASK3A | Timer-based LED pattern generator and button interrupt controller |
| 8051 | CAT4 | TASK4A | Keypad-based LED and seven-segment controller |
| 8051 | CAT5 | TASK5A | Hardware timer, PWM, pulse and optional stepper control |
| 8051 | CAT6 | TASK6A | UART driver and serial debugging |
| 8051 | CAT7 | TASK7A-TASK7C | 16x2 LCD driver, dynamic display and keypad character movement |
| STM32 | CAT8 | TASK8A, TASK8B, TASK8C | GPIO input/output, debounced event toggle and short/long press control |
| STM32 | CAT9 | TASK9A, TASK9B | Basic UART transmit/echo and string-based LED commands |
| STM32 | CAT10 | TASK10A, TASK10B, TASK10C | Wemos UART, ESP8266 AT commands, Wi-Fi and BME280 TCP transmission |
| STM32 | CAT11 | TASK11A, TASK11B | On-board LED, switch state remembrance and short/long press detection |
| STM32 | CAT12 | TASK12A, TASK12B, TASK12C | Bosch BME280 SensorAPI, HTTP/JSON output and Node-RED analytics |
| STM32 | CAT13 | TASK13 | BME280 readings with custom ESP8266 MQTT publish/subscribe API |

## Notes

Project files and source code are retained for evaluation. Keil-generated intermediate files are excluded according to the supplied `.gitignore`; HEX firmware files are intentionally tracked.
