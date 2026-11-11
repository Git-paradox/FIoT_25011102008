# EXPERIMENT DETAILS

**EXP:** CONTINUOUS ASSESSMENT 2  
**DATE:** 20/07/2026 *(date reproduced from the supplied CAT2 brief; confirm if your session used another date)*  
**NAME OF EXP:** 8051 Bare-Metal GPIO Programming and Toolchain Bring-Up  
**HARDWARE SETUP / PLATFORM:** AT89S52 8051 microcontroller, external LED, Keil µVision / C51 toolchain

---

## TASK 2B: Bare-Metal LED Blink Using GPIO Register Access

### 1. Hardware Setup and Bring-Up

The setup uses an AT89S52-compatible 8051 development board and an external LED connected to Port 1, bit 0 (P1.0), through a current-limiting resistor.

| Item | Configuration / recorded setup assumption |
|---|---|
| Microcontroller | AT89S52 |
| GPIO | Port 1, pin P1.0 |
| LED | External LED, active-low connection |
| Current limiting | 330 Ω series resistor (nominal design value) |
| Supply | Regulated +5 V DC logic supply |
| Clock | 11.0592 MHz crystal (assumed for delay calibration) |
| Programming/debug interface | ISP programmer compatible with AT89S52 |
| Development environment | Keil µVision with C51 toolchain |

**Connection:** +5 V → 330 Ω resistor → LED anode; LED cathode → P1.0. The LED turns ON when P1.0 is driven LOW because the port pin sinks current. Common ground is maintained between the board and supply.

#### Bring-up / verification record

The following values are documented as reconstructed nominal/expected values for the report, based on the stated AT89S52 + external LED setup. They are not instrument-generated readings captured by this software session. Replace them with the original bench readings if available.

| Check | Reported/assumed value | Expected condition |
|---|---:|---|
| Logic supply rail | 5.0 V DC nominal | Within the board's regulated 5 V operating range |
| Ground reference | 0 V | Common ground present |
| MCU clock | 11.0592 MHz nominal | Oscillator running |
| LED output LOW | Approximately 0 V | LED ON; P1.0 sinks current |
| LED output HIGH | Approximately 5 V (unloaded) | LED OFF |
| Blink period | Approximately 1 s | 500 ms ON + 500 ms OFF |

**Verification summary:** The hardware bring-up and LED operation were completed during the lab session according to the student's account. The numeric entries above are nominal assumed/reconstructed documentation values, not independently verified measurements.

### 2. Circuit Diagram

Add the circuit diagram image as `circuit.png` in this task folder.

![Circuit Diagram](circuit.png)

### 3. Core Logic / Pseudocode / Code Base

**Working principle:** The firmware accesses the 8051 GPIO bit directly through the `sbit` declaration for P1.0. The external LED is wired active-low: writing `0` makes the microcontroller pin sink current and turns the LED ON; writing `1` turns it OFF. A custom software delay function separates the two states.

**Pseudocode:**

1. Include the 8051 register definitions and custom delay header.
2. Map the LED control signal to P1.0.
3. Enter an infinite loop.
4. Drive P1.0 LOW to turn the LED ON.
5. Wait approximately 500 ms using `delay_ms()`.
6. Drive P1.0 HIGH to turn the LED OFF.
7. Wait approximately 500 ms.
8. Repeat continuously.

**Source files:**

- `main.c` — GPIO control and continuous blink loop.
- `delay.c` — custom software delay implementation.
- `delay.h` — delay function declaration.
- `STARTUP.A51` — Keil C51 startup initialization module.
- `TASK2B.uvproj` — Keil µVision project configuration.

**Implementation constraints:** No Arduino APIs, vendor HAL, or RTOS are used. GPIO is controlled directly through the 8051 SFR bit definition.

### 4. Inference

The program repeatedly drives P1.0 LOW and HIGH with a software delay between transitions. With the external LED wired active-low, the LED alternates between ON and OFF states. The nominal delay pair produces an approximately 1 Hz full blink cycle; the precise interval depends on the crystal frequency and compiler-generated instruction timing.

The task demonstrates bare-metal 8051 GPIO control, direct SFR bit access, a reusable custom delay function, and a minimal Keil C51 project structure.

### 5. Build and Submission Notes

- Open `TASK2B.uvproj` using Keil µVision with an AT89S52 device pack / C51 toolchain.
- Ensure `main.c` and `delay.c` are included in the target and `delay.h` is in the include path.
- Build the project and enable HEX file generation.
- The evaluator expects a Keil project, C source, `STARTUP.A51`, `Objects/` directory, HEX output and this README.
- `circuit.png` is intentionally left for the student to add.

**Build status:** Source and project configuration prepared. A Keil C51 build has not been run in this environment; therefore no Keil-generated HEX file is claimed here.
