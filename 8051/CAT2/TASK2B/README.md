# EXPERIMENT DETAILS

**EXP:** CONTINUOUS LAB EVALUATION - 2  
**DATE:** 20/07/2026 (Week 2)  
**COURSE:** 25CS2741 - Fundamentals of Internet of Things + Lab  
**NAME OF EXP:** 8051 Tool Chain Setup - Development Board - AT89S52  
**HARDWARE SETUP / PLATFORM:** AT89S52 8051 development board

---

## Activity: Bare-Metal LED Blink

Compile a minimal bare-metal C program, program the device, and control an LED by directly accessing the microcontroller's memory-mapped GPIO registers.

**Constraint:** No Arduino APIs, HAL libraries, or RTOS services are used.

### 1. Hardware Bring-Up Sheet

The lab board was brought up and tested during the practical session, as confirmed by the student. The exact multimeter readings and programmer model were not retained in the notes. The following bring-up entries are therefore documented as reasonable nominal assumptions for the AT89S52 setup and should not be interpreted as exact archived meter readings.

| Parameter | Student entry / documented assumption |
|---|---|
| MCU | AT89S52 |
| CPU Core | 8-bit MCS-51 / 8051-compatible core |
| Operating Voltage | 5.0 V nominal; device operating range 4.0 V to 5.5 V |
| Debug Interface | SPI-based ISP programmer (USB programmer assumed; exact model not recorded) |
| LED GPIO Port | Port 1 (P1) |
| LED GPIO Pin | P1.0 |
| LED Active State | LOW (active-low wiring) |
| LED Frequency (Hz) | Approximately 1 Hz (one complete ON-OFF cycle per second) |

**Power and wiring assumptions used for documentation:**

- Supply rail: +5 V DC nominal.
- Ground: common 0 V reference.
- LED connection: +5 V → 330 Ω current-limiting resistor → LED anode; LED cathode → P1.0.
- P1.0 LOW sinks current and turns the LED ON; P1.0 HIGH turns it OFF.
- Crystal frequency assumed for delay-loop calibration: 11.0592 MHz.

The AT89S52 datasheet specifies a 4.0 V to 5.5 V operating range and identifies the device as an 8-bit 8051-compatible microcontroller. The board-level 5.0 V rail and 11.0592 MHz clock above are nominal setup assumptions, not claimed exact DMM or oscilloscope measurements.

### 2. Hardware Bring-Up Procedure

1. Identify the AT89S52 MCU and the board's supply, ground, oscillator and programming connections.
2. Apply the regulated 5 V supply and verify the supply rail relative to ground using a digital multimeter.
3. Identify P1.0 from the MCU pin diagram / board schematic.
4. Connect the external LED and series resistor in the active-low configuration described above.
5. Connect the host computer to the MCU using an SPI ISP programmer.
6. Confirm that the programmer can communicate with the target before programming the firmware.

### 3. Circuit Diagram

Add the circuit diagram image as `circuit.png` in this task folder.

![Circuit Diagram](circuit.png)

### 4. Core Logic / Pseudocode / Code Base

**Working principle:** The firmware directly controls the P1.0 port bit. The external LED is wired active-low, so writing 0 to P1.0 turns it ON and writing 1 turns it OFF. A custom software delay function separates the two states.

**Pseudocode:**

1. Include the 8051 register definition and custom delay header.
2. Associate the LED with P1.0.
3. Enter an infinite loop.
4. Write LOW to P1.0 to turn the LED ON.
5. Call the custom delay function for approximately 500 ms.
6. Write HIGH to P1.0 to turn the LED OFF.
7. Call the custom delay function for approximately 500 ms.
8. Repeat continuously.

**Source files:**

- `main.c` — direct GPIO control and continuous LED blink loop.
- `delay.c` — custom software delay implementation using nested loops.
- `delay.h` — delay function declaration.
- `STARTUP.A51` — Keil C51 startup initialization module.
- `TASK2B.uvproj` — Keil µVision project configured for the AT89S52.

**Implementation constraints:** The project uses bare-metal Embedded C and direct SFR/bit access. It does not use Arduino APIs, HAL libraries, or RTOS services.

### 5. Inference

The program repeatedly drives P1.0 LOW and HIGH with a software delay between transitions. With the external LED wired active-low, the LED alternates between ON and OFF states. The target is an approximately 1 Hz blink frequency, corresponding to an ON interval and an OFF interval of roughly 500 ms each.

The experiment demonstrates 8051 toolchain setup, basic hardware bring-up, GPIO output configuration through direct register access, and implementation of a reusable custom delay library function.

### 6. Build and Submission Notes

- Open `TASK2B.uvproj` in Keil µVision with the Keil C51 toolchain and AT89S52 device support.
- Ensure `main.c` and `delay.c` are included in the target; `delay.h` is included from the task directory.
- Build the target and enable Intel HEX output.
- The expected task structure includes the Keil project, source file(s), `STARTUP.A51`, `Objects/`, an `Objects/*.hex` firmware image, this README, and `circuit.png`.
- `circuit.png` is intentionally left for the student to add.
- The software delay is approximate and is compiler/optimization dependent; the loop count should be calibrated against the final board clock and measured blink rate.

**Build status:** GitHub Actions successfully compiled the source with SDCC 4.5.0 for the MCS-51 target and generated `Objects/TASK2B.hex` in Intel HEX format. The Keil µVision project and `STARTUP.A51` are included for the requested Keil workflow, but a Keil C51 build itself has not been run or claimed. The generated HEX is an SDCC build, not a Keil C51 build.
