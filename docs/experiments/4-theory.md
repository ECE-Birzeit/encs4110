---
outline: deep
lastUpdated: true
prev:
  text: "3. Control Flow and Subroutines - Procedure"
  link: /experiments/3-procedure
next:
  text: "4. Microcontroller Architecture and GPIO Output - Procedure"
  link: /experiments/4-procedure
---
# Experiment 4: Microcontroller Architecture and GPIO Output
::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 <a href="../manual/encs4110_4.pdf" target="_blank">Download Experiment 4 PDF</a>
:::

## Learning Objectives

After completing this experiment, you will be able to:

- Identify the main components and subsystems of the TM4C123 microcontroller and understand their roles within an embedded system.
- Recognize the organization of the ARM Cortex-M4 core, memory map, and peripheral buses.
- Understand the role of core peripherals including NVIC, SysTick, and System Control Block (SCB).
- Configure and control digital output pins using the General-Purpose Input/Output (GPIO) module.
- Write Assembly programs to drive on-board LEDs through direct register access.
- Relate register operations to physical hardware behavior and verify functionality through observation on the LaunchPad board.

## Experiment Overview

This experiment introduces the TM4C123 microcontroller architecture and digital I/O interfacing through GPIO ports. You will explore the internal structure including the ARM Cortex-M4 core, memory regions, and peripheral interconnect, then write Assembly programs to control the on-board LEDs connected to PORTF. By the end of this lab, you will understand how peripheral registers are memory-mapped, be able to initialize and configure GPIO ports, and control external hardware through register manipulation—forming the foundation for subsequent experiments with interrupts, timers, and analog peripherals.



## Theoretical Background

### TM4C123 Microcontroller Architecture

The TM4C123GH6PM microcontroller is built around the ARM Cortex-M4F processor core, implementing the ARMv7-M architecture. It integrates the CPU core, memory, peripherals, and I/O interfaces on a single chip — a complete system-on-chip (SoC) solution for embedded applications.

#### Core Components

The TM4C123 contains the following major components:

- **ARM Cortex-M4F Processor Core**: 32-bit RISC processor with hardware Floating-Point Unit (FPU)
- **Memory**: 256 KB Flash, 32 KB SRAM, 2 KB EEPROM
- **System Buses**: Advanced High-Performance Bus (AHB) and Advanced Peripheral Bus (APB)
- **Core Peripherals**: NVIC, SysTick Timer, Memory Protection Unit (MPU), System Control Block (SCB)
- **General-Purpose Timers**: Six 16/32-bit and six 32/64-bit timers
- **Communication Interfaces**: UART, I²C, SSI (SPI), CAN
- **Analog Peripherals**: 12-bit ADC with 12 channels, analog comparators
- **GPIO Ports**: Six ports (A-F) with up to 43 programmable pins

### ARM Cortex-M4 Core Peripherals

The ARM Cortex-M4 processor includes several **core peripherals** that are common across all Cortex-M4-based microcontrollers. They are tightly integrated with the CPU and provide essential system functions such as interrupt handling and timing.

#### Nested Vectored Interrupt Controller (NVIC)

The NVIC manages all interrupt and exception handling. It supports up to 240 interrupt sources (138 on the TM4C123), hardware priority levels, and automatic context saving for efficient servicing. Key features include nesting, tail-chaining, and late-arrival handling for minimal interrupt latency.

Important NVIC registers (base address `0xE000E100`):

- `NVIC_ENx` — Set-Enable
- `NVIC_DISx` — Clear-Enable
- `NVIC_PRIx` — Priority Configuration
- `NVIC_ACTIVEx` — Active Status

#### SysTick Timer

The SysTick timer is a simple 24-bit down-counter integrated in the core. It provides a consistent time base for system delays or periodic tasks. Many operating systems use it for time-keeping or scheduling.

SysTick registers (base address `0xE000E010`):

- `STCTRL` — Control and Status
- `STRELOAD` — Reload Value
- `STCURRENT` — Current Counter Value

### Memory Map

The Cortex-M4 processor uses a unified 4 GB address space for all code, data, and peripherals. Every peripheral and memory region occupies a unique address range, allowing direct access through normal load and store instructions.

| Region | Address Range | Description |
|--------|---------------|-------------|
| Flash Memory | 0x00000000 - 0x0003FFFF | Program storage (256 KB) |
| SRAM | 0x20000000 - 0x20007FFF | On-chip data memory (32 KB) |
| Peripherals | 0x40000000 - 0x400FFFFF | Peripheral registers |
| GPIO Ports | 0x40004000 - 0x40025FFF | GPIO A-F registers |
| Core Peripherals | 0xE0000000 - 0xE00FFFFF | NVIC, SysTick, SCB, etc. |

Each peripheral's registers are **memory-mapped**, meaning they are accessed just like variables in memory. For example, writing to address `0x400253FC` directly updates the GPIO Port F data register.

### General-Purpose Input/Output (GPIO)

GPIO (General-Purpose Input/Output) ports form the primary interface between the microcontroller and external devices such as LEDs, switches, and sensors. Each pin can be configured as either an input or an output.

When a pin is an **output**, software drives it high or low to control external hardware (e.g., turn an LED on/off). When a pin is an **input**, software reads its logic level (e.g., a button press).

#### GPIO Ports Overview

The TM4C123GH6PM provides six GPIO ports (A-F), each with up to eight programmable pins. Not all pins are available on the LaunchPad, and some are reserved for debugging or special functions.

| Port | Pins Available | Notes |
|------|----------------|-------|
| Port A | PA0-PA7 | UART0 (PA0, PA1) shared with USB debug interface |
| Port B | PB0-PB7 | I²C0, SSI2, ADC channels |
| Port C | PC0-PC7 | PC0-PC3 used for JTAG (avoid modification) |
| Port D | PD0-PD7 | PD7 requires unlock for GPIO use |
| Port E | PE0-PE5 | ADC and UART5 functionality available |
| Port F | PF0-PF4 | On-board LEDs (PF1-PF3) and switches (PF0, PF4) |

#### Memory-Mapped GPIO Registers

GPIO modules are accessed via **memory-mapped registers**: fixed addresses that the CPU reads/writes with standard `LDR`/`STR` instructions.

Every port has a **base address**; key registers sit at fixed offsets from that base.

| Port | Base Address |
|------|--------------|
| Port A | 0x40004000 |
| Port B | 0x40005000 |
| Port C | 0x40006000 |
| Port D | 0x40007000 |
| Port E | 0x40024000 |
| Port F | 0x40025000 |

In this experiment we use Port F, so:

```
GPIODIR = 0x40025400,
GPIODEN = 0x4002551C,
GPIODATA = 0x400253FC.
```

#### Address Masking in `GPIODATA`

The `GPIODATA` register supports **address masking**: address bits [9:2] form a bit mask that selects which pins are affected.

- **Full access:** Base + `0x3FC` — all 8 pins.
- **Masked access:** Base + (`mask` << 2) — only the pins in `mask`.

Example (PF1 only):
```
0x40025000 + (0x02 << 2) = 0x40025008
```
This is **address masking** (often confused with "bit-banding"); it enables fast, selective reads/writes to individual pins.

#### Protected and Special-Function Pins

Some pins are locked or reserved at reset due to critical roles:

- **PF0** doubles as the Non-Maskable Interrupt (NMI) input and is locked. To use it as GPIO, write `0x4C4F434B` to `GPIO_LOCK` and set the desired bits in `GPIO_CR`.
- **PC0-PC3** carry the JTAG debug interface and should not be reassigned in normal operation.

#### Port F on the LaunchPad

Port F connects to the on-board RGB LEDs and user push buttons:

| Pin | Function | Connected Component | Active Level |
|-----|----------|-------------------|--------------|
| PF1 | Output | Red LED | Active High |
| PF2 | Output | Blue LED | Active High |
| PF3 | Output | Green LED | Active High |
| PF0 | Input (locked) | SW2 (Push Button) | Active Low (enable pull-up) |
| PF4 | Input | SW1 (Push Button) | Active Low (enable pull-up) |

LEDs are **active-high** (write 1 to turn on). Push buttons pull to ground when pressed, so inputs read **0 when pressed** and require internal pull-ups.

#### GPIO Configuration Workflow

In practice, GPIO usage has two phases:

1. **Initialization (setup, runs once):** enable the port clock, unlock protected pins (if needed), set direction, and enable digital function.
2. **Runtime (operation, repeats):** read inputs or write outputs by accessing `GPIODATA` in the main loop or in interrupt service routines (ISRs).

### Step 1 — Enable the GPIO Port Clock

**Register:** `RCGCGPIO` — Run Mode Clock Gating Control for GPIO (`0x400FE608`)

Before accessing any GPIO port, its clock must be enabled. The `RCGCGPIO` register controls the clock to all GPIO modules. Each bit corresponds to a single port (A-F). Writing a '1' to a bit enables the clock to that port, while '0' disables it.

To activate Port F, bit 5 must be set to 1.

```asm
        LDR     R1, =0x400FE608     ; RCGCGPIO register
        LDR     R0, [R1]
        ORR     R0, R0, #0x20       ; Set bit 5 (Port F)
        STR     R0, [R1]
```

### Step 2 — Unlock Protected Pins

**Registers:** `GPIO_LOCK` (`0x40025520`), `GPIO_CR` (`0x40025524`)

Certain pins such as PF0 are protected because they share critical alternate functions (e.g., the NMI input). To modify these pins, the port must be unlocked by writing the key value `0x4C4F434B` ("LOCK") into the `GPIO_LOCK` register. The `GPIO_CR` register (Commit Register) then determines which pins can be altered.

```asm
        LDR     R1, =0x40025520     ; GPIO_LOCK
        LDR     R0, =0x4C4F434B     ; Unlock key
        STR     R0, [R1]
        LDR     R1, =0x40025524     ; GPIO_CR
        MOV     R0, #0xFF           ; Allow changes to all pins
        STR     R0, [R1]
```

### Step 3 — Configure Pin Direction

**Register:** `GPIODIR` (`0x40025400`) — GPIO Direction Control (Port F)

The `GPIODIR` register determines whether each GPIO pin functions as an input or an output. Writing '0' configures the pin as an input, while '1' configures it as an output.

For the on-board RGB LED, PF1-PF3 must be configured as outputs. Bits 1-3 are therefore set to '1'.

```asm
        LDR     R1, =0x40025400     ; GPIODIR register (Port F)
        LDR     R0, [R1]
        ORR     R0, R0, #0x0E       ; Set PF1, PF2, PF3 as outputs
        STR     R0, [R1]
```

### Step 4 — Enable Digital Functionality

**Register:** `GPIODEN` (`0x4002551C`) — Digital Enable Register (Port F)

Each GPIO pin can serve analog or digital functions. The `GPIODEN` register enables the digital circuitry for selected pins. Pins configured as digital inputs or outputs must have their corresponding bits set to '1'.

Bits 1-3 are set to enable PF1-PF3 as digital outputs for the LED.

```asm
        LDR     R1, =0x4002551C     ; GPIODEN register (Port F)
        LDR     R0, [R1]
        ORR     R0, R0, #0x0E       ; Enable PF1-PF3 as digital pins
        STR     R0, [R1]
```

### Step 5 — Write to the Data Register

**Register:** `GPIODATA` (`0x400253FC`) — Data Input/Output Register (Port F)

The `GPIODATA` register reflects the current logic levels on all GPIO pins. Writing to a bit drives the corresponding output high ('1') or low ('0'). Bits 1-3 correspond to the RGB LED pins on the LaunchPad: PF1 = Red, PF2 = Blue, PF3 = Green.

The example below drives PF1 (Red) and PF3 (Green) simultaneously to produce a yellow color.

```asm
    LDR     R1, =0x400253FC     ; GPIODATA register (Port F)
    MOV     R0, #0x0A           ; Set PF1 (Red) and PF3 (Green) = Yellow
    STR     R0, [R1]
```

#### LED Color Combinations

| Color | Red (PF1) | Blue (PF2) | Green (PF3) | Hex | Combination |
|-------|-----------|------------|-------------|-----|-------------|
| Red | ON | OFF | OFF | 0x02 | Red only |
| Blue | OFF | ON | OFF | 0x04 | Blue only |
| Green | OFF | OFF | ON | 0x08 | Green only |
| Yellow | ON | OFF | ON | 0x0A | Red + Green |
| Cyan | OFF | ON | ON | 0x0C | Blue + Green |
| Magenta | ON | ON | OFF | 0x06 | Red + Blue |
| White | ON | ON | ON | 0x0E | All on |