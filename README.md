# Foundations of IoT — Lab Submissions

Repository for recording Foundations of IoT continuous assessment tasks.

## Platforms

- `8051/` — 8051 microcontroller tasks
- `STM32/` — STM32 microcontroller tasks

Each task is stored under its corresponding CAT and TASK directory and includes a task-specific `README.md` and circuit diagram (`circuit.png`, added by the student).

## Current submissions

| Platform | CAT | Task | Description |
|---|---|---|---|
| 8051 | CAT2 | TASK2B | Bare-metal GPIO LED blink with custom delay function |
| 8051 | CAT3 | TASK3A | Timer-based LED pattern generator and button interrupt controller |
| 8051 | CAT4 | TASK4A | Keypad-based LED and seven-segment controller |
| 8051 | CAT5 | TASK5A | Hardware timer, PWM, pulse and optional stepper control |
| 8051 | CAT6 | TASK6A | UART driver and serial debugging |
| 8051 | CAT7 | TASK7A-TASK7C | 16x2 LCD driver, dynamic display and keypad character movement |
| STM32 | CAT8 | TASK8A-TASK8C | GPIO input/output, event-based toggle and short/long press control |
| STM32 | CAT9 | TASK9A-TASK9B | UART communication and string-based LED commands |
| STM32 | CAT10 | TASK9A-TASK9C | Wemos ESP8266 AT commands, Wi-Fi connection and BME280 data transmission |

## Notes

Project files and source code are retained for evaluation. Keil-generated intermediate files are excluded according to the supplied `.gitignore`; HEX firmware files are intentionally tracked.
