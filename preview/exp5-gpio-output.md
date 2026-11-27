---
title: "Microcontroller Architecture and GPIO Output"
description: "This experiment introduces the TM4C123 microcontroller architecture and digital I/O interfacing through GPIO ports."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.5"
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=649"
current: "/experiments/exp5-gpio-output"
short: "GPIO Output"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Microcontroller Architecture and GPIO Output

This experiment introduces the TM4C123 microcontroller architecture and digital I/O interfacing through GPIO ports. You will explore the internal structure including the ARM Cortex-M4 core, memory regions, and peripheral interconnect, then write Assembly programs to control the on-board LEDs connected to PORTF. By the end of this lab, you will understand how peripheral registers are memory-mapped, be able to initialize and configure GPIO ports, and control external hardware through register manipulation—forming the foundation for subsequent experiments with interrupts, timers, and analog peripherals.

::: warning Reworked version: in testing
This is the reworked version of [Experiment 5](/experiments/exp5-gpio-output), shared for TAs to test. It has the same background, then one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp5-gpio-output)** until this one replaces it.
:::


## Objectives

- Identify the main components and subsystems of the TM4C123 microcontroller and understand their roles within an embedded system.
- Recognize the organization of the ARM Cortex-M4 core, memory map, and peripheral buses.
- Understand the role of core peripherals including NVIC, SysTick, and System Control Block (SCB).
- Configure and control digital output pins using the General-Purpose Input/Output (GPIO) module.
- Write Assembly programs to drive on-board LEDs through direct register access.
- Relate register operations to physical hardware behavior and verify functionality through observation on the LaunchPad board.

## Background

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

Other core peripherals such as the System Control Block (SCB) are present but not covered in this course.

This experiment’s own examples and tasks use neither the NVIC nor SysTick — they’re introduced here as groundwork you’ll configure hands-on in Experiment 6 (NVIC-driven GPIO interrupts) and Experiment 7 (SysTick-based timing).

### Memory Map

The Cortex-M4 processor uses a unified 4 GB address space for all code, data, and peripherals. Every peripheral and memory region occupies a unique address range, allowing direct access through normal load and store instructions.

| **Region**       | **Address Range**       | **Description**             |
|:-----------------|:------------------------|:----------------------------|
| Flash Memory     | 0x00000000 - 0x0003FFFF | Program storage (256 KB)    |
| SRAM             | 0x20000000 - 0x20007FFF | On-chip data memory (32 KB) |
| Peripherals      | 0x40000000 - 0x400FFFFF | Peripheral registers        |
| GPIO Ports       | 0x40004000 - 0x40025FFF | GPIO A-F registers          |
| Core Peripherals | 0xE0000000 - 0xE00FFFFF | NVIC, SysTick, SCB, etc.    |

*Simplified TM4C123 Memory Map*

Each peripheral’s registers are **memory-mapped**, meaning they are accessed just like variables in memory. For example, writing to address `0x400253FC` directly updates the GPIO Port F data register.

### General-Purpose Input/Output (GPIO)

GPIO (General-Purpose Input/Output) ports form the primary interface between the microcontroller and external devices such as LEDs, switches, and sensors. Each pin can be configured as either an input or an output.

When a pin is an **output**, software drives it high or low to control external hardware (e.g., turn an LED on/off). When a pin is an **input**, software reads its logic level (e.g., a button press).

#### GPIO Ports Overview

The TM4C123GH6PM provides six GPIO ports (A-F), each with up to eight programmable pins. Not all pins are available on the LaunchPad, and some are reserved for debugging or special functions.

| **Port** | **Pins Available** | **Notes** |
|:--:|:---|:---|
| Port A | PA0-PA7 | UART0 (PA0, PA1) shared with USB debug interface |
| Port B | PB0-PB7 | I²C0, SSI2, ADC channels |
| Port C | PC0-PC7 | PC0-PC3 used for JTAG (avoid modification) |
| Port D | PD0-PD7 | PD7 requires unlock for GPIO use |
| Port E | PE0-PE5 | ADC and UART5 functionality available |
| Port F | PF0-PF4 | On-board LEDs (PF1-PF3) and switches (PF0, PF4) |

*GPIO Port Summary — TM4C123GH6PM*

Each port exposes its own control and data registers, allowing independent configuration and operation.

#### Memory-Mapped GPIO Registers

GPIO modules are accessed via **memory-mapped registers**: fixed addresses that the CPU reads/writes with standard `LDR`/`STR` instructions.

Every port has a **base address**; key registers sit at fixed offsets from that base.

| **Port** | **Base Address** |
|:--------:|:-----------------|
|  Port A  | 0x40004000       |
|  Port B  | 0x40005000       |
|  Port C  | 0x40006000       |
|  Port D  | 0x40007000       |
|  Port E  | 0x40024000       |
|  Port F  | 0x40025000       |

*GPIO Port Base Addresses (TM4C123GH6PM)*

In this experiment we use Port F, so:
$$
\texttt{GPIODIR} = \texttt{0x40025400},\quad
\texttt{GPIODEN} = \texttt{0x4002551C},\quad
\texttt{GPIODATA} = \texttt{0x400253FC}.
$$

#### Address Masking in `GPIODATA`

The `GPIODATA` register supports **address masking**: address bits \[9:2\] form a bit mask that selects which pins are affected.

- **Full access:** Base + `0x3FC` — all 8 pins.
- **Masked access:** Base + (`mask` $\ll$ 2) — only the pins in `mask`.

Example (PF1 only):
$$
\texttt{0x40025000 + (0x02 << 2) = 0x40025008}.
$$
This is **address masking** (often confused with “bit-banding”); it enables fast, selective reads/writes to individual pins.

#### Protected and Special-Function Pins

Some pins are locked or reserved at reset due to critical roles:

- **PF0** doubles as the Non-Maskable Interrupt (NMI) input and is locked. To use it as GPIO, write `0x4C4F434B` to `GPIO_LOCK` and set the desired bits in `GPIO_CR`.
- **PC0-PC3** carry the JTAG debug interface and should not be reassigned in normal operation.

Always confirm pin availability in the datasheet before repurposing special-function pins.

#### Port F on the LaunchPad

Port F connects to the on-board RGB LEDs and user push buttons:

| **Pin** | **Function**   | **Connected Component** | **Active Level**            |
|:-------:|:---------------|:------------------------|:----------------------------|
|   PF1   | Output         | Red LED                 | Active High                 |
|   PF2   | Output         | Blue LED                | Active High                 |
|   PF3   | Output         | Green LED               | Active High                 |
|   PF0   | Input (locked) | SW2 (Push Button)       | Active Low (enable pull-up) |
|   PF4   | Input          | SW1 (Push Button)       | Active Low (enable pull-up) |

*GPIO Port F Pin Assignments — TM4C123 LaunchPad*

LEDs are **active-high** (write 1 to turn on). Push buttons pull to ground when pressed, so inputs read **0 when pressed** and require internal pull-ups.

#### GPIO Configuration Workflow

In practice, GPIO usage has two phases:

- **Initialization (setup, runs once):** enable the port clock, unlock protected pins (if needed), set direction, and enable digital function.
- **Runtime (operation, repeats):** read inputs or write outputs by accessing `GPIODATA` in the main loop or in interrupt service routines (ISRs).

A typical structure is:

```c
InitGPIO();                // RCGCGPIO, optional LOCK/CR, DIR, DEN
while (1) {                // Runtime phase
    // Read buttons (inputs) and/or drive LEDs (outputs)
    // Optionally, some I/O is handled inside ISRs
}
```

The detailed register-level steps (clock enable, unlock, direction, digital enable, and data access) are demonstrated in the following subsections.

#### 1. Enable the GPIO Port Clock

**Register:** `RCGCGPIO` — Run Mode Clock Gating Control for GPIO (`0x400FE608`)

Before accessing any GPIO port, its clock must be enabled. The `RCGCGPIO` register controls the clock to all GPIO modules. Each bit corresponds to a single port (A-F). Writing a ’1’ to a bit enables the clock to that port, while ’0’ disables it.

**RCGCGPIO Register (0x400FE608) — GPIO Clock Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–6 | reserved |
| 5 | PF |
| 4 | PE |
| 3 | PD |
| 2 | PC |
| 1 | PB |
| 0 | PA |

*To activate Port F, bit 5 must be set to 1 as shown below.*

```asm
LDR     R1, =0x400FE608     ; RCGCGPIO register
LDR     R0, [R1]
ORR     R0, R0, #0x20       ; Set bit 5 (Port F)
STR     R0, [R1]
```

Once the clock is enabled, a short delay or status polling ensures the peripheral is ready before further configuration. This delay exists because enabling a peripheral’s clock in `RCGCGPIO` doesn’t take effect instantaneously — the clock-gating logic needs a few cycles to settle before the port’s registers can be safely accessed. A couple of `NOP` instructions (as used in this experiment’s examples) are enough to bridge that gap; the exact count isn’t critical as long as some delay follows the clock-enable write. A more robust alternative — used from [Experiment 6](/preview/exp6-gpio-interrupts) on — is to poll a peripheral-ready register (`SYSCTL->PRGPIO`) instead of guessing a fixed number of `NOP`s.

#### 2. Unlock Protected Pins

**Registers:** `GPIO_LOCK` (`0x40025520`), `GPIO_CR` (`0x40025524`)

Certain pins such as PF0 are protected because they share critical alternate functions (e.g., the NMI input). To modify these pins, the port must be unlocked by writing the key value `0x4C4F434B` (“LOCK”) into the `GPIO_LOCK` register. The `GPIO_CR` register (Commit Register) then determines which pins can be altered.

**GPIO_CR Register — Commit Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7–0 | Commit Mask |

*The following code unlocks Port F and enables modification of all pins.*

```asm
LDR     R1, =0x40025520     ; GPIO_LOCK
LDR     R0, =0x4C4F434B     ; Unlock key
STR     R0, [R1]            
LDR     R1, =0x40025524     ; GPIO_CR
MOV     R0, #0xFF           ; Allow changes to all pins
STR     R0, [R1]
```

In this experiment, PF0 is not used, so unlocking is optional. After unlocking, configuration registers such as direction and digital enable can be safely modified.

#### 3. Configure Pin Direction

**Register:** `GPIODIR` (`0x40025400`) — GPIO Direction Control (Port F)

The `GPIODIR` register determines whether each GPIO pin functions as an input or an output. Writing ’0’ configures the pin as an input, while ’1’ configures it as an output.

**GPIODIR Register (Port F) — Direction Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

For the on-board RGB LED, PF1-PF3 must be configured as outputs. Bits 1-3 are therefore set to ’1’.

```asm
LDR     R1, =0x40025400     ; GPIODIR register (Port F)
LDR     R0, [R1]
ORR     R0, R0, #0x0E       ; Set PF1, PF2, PF3 as outputs
STR     R0, [R1]
```

Unmodified bits remain unchanged, allowing input pins (such as PF0 or PF4) to retain their default configuration.

#### 4. Enable Digital Functionality

**Register:** `GPIODEN` (`0x4002551C`) — Digital Enable Register (Port F)

Each GPIO pin can serve analog or digital functions. The `GPIODEN` register enables the digital circuitry for selected pins. Pins configured as digital inputs or outputs must have their corresponding bits set to ’1’.

**GPIODEN Register (Port F) — Digital Enable**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

*Bits 1-3 are set to enable PF1-PF3 as digital outputs for the LED.*

```asm
LDR     R1, =0x4002551C     ; GPIODEN register (Port F)
LDR     R0, [R1]
ORR     R0, R0, #0x0E       ; Enable PF1-PF3 as digital pins
STR     R0, [R1]
```

Failing to enable `GPIODEN` leaves the pins electrically inactive even if their direction is set.

#### 5. Write to the Data Register

**Register:** `GPIODATA` (`0x400253FC`) — Data Input/Output Register (Port F)

The `GPIODATA` register reflects the current logic levels on all GPIO pins. Writing to a bit drives the corresponding output high (’1’) or low (’0’). Bits 1-3 correspond to the RGB LED pins on the LaunchPad: PF1 = Red, PF2 = Blue, PF3 = Green.

**GPIODATA Register (Port F) — Data Output/Input**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | PF7 |
| 6 | PF6 |
| 5 | PF5 |
| 4 | PF4 |
| 3 | PF3 |
| 2 | PF2 |
| 1 | PF1 |
| 0 | PF0 |

The example below drives PF1 (Red) and PF3 (Green) simultaneously to produce a yellow color.

```asm
LDR     R1, =0x400253FC     ; GPIODATA register (Port F)
MOV     R0, #0x0A           ; Set PF1 (Red) and PF3 (Green) = Yellow
STR     R0, [R1]
```

By writing different bit combinations, various LED colors can be generated:

| **Color** | **Red (PF1)** | **Blue (PF2)** | **Green (PF3)** | **Hex** | **Combination** |
|:---|:--:|:--:|:--:|:--:|:---|
| Red | on | – | – | 0x02 | Red only |
| Blue | – | on | – | 0x04 | Blue only |
| Green | – | – | on | 0x08 | Green only |
| Yellow | on | – | on | 0x0A | Red + Green |
| Cyan | – | on | on | 0x0C | Blue + Green |
| Magenta | on | on | – | 0x06 | Red + Blue |
| White | on | on | on | 0x0E | All on |

*LED Color Combinations on TM4C123 LaunchPad (PF1-PF3)*

## Activity

Cycle the RGB LED through all seven colours it can make: red, yellow, green, cyan, blue, magenta, and white, about a third of a second each, forever. The program is in assembly, as in Part 1, and uses `EQU` names for every address and bit, so it reads as what it does rather than as hex numbers. You write `main.s` in three parts, top to bottom.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **assembly**, and make sure its blinking-LED test works. All the code goes in that project's `main.s`.
:::

Nothing needs wiring: the LED is on the board. `SystemInit` in the startup code sets the clock to 50 MHz before `__main` runs.

### Step 1: Add the definitions

Replace the contents of `main.s` with the names and the file header:

<<< @/snippets/preview/exp5/main.asm#defines

- **`EQU`** gives a number a name, like `#define` in C. It reserves no memory: the assembler replaces the name with its value wherever it appears.
- **`GPIOF_RGB`** is Port F's `DATA` address with the PF1-PF3 mask from [Address Masking in `GPIODATA`](#address-masking-in-gpiodata) added: a write there changes only the three LED pins, whatever the other bits say.
- The colours are sums of the LED bits: yellow is red and green together. They come from the [colour table](#port-f-on-the-launchpad) above.
- `AREA`, `THUMB`, and `EXPORT __main` are the same header as the project's test program.

### Step 2: Add the subroutines

Add three subroutines after the header:

<<< @/snippets/preview/exp5/main.asm#functions

- **`PF_Init`** is the [GPIO configuration](#gpio-configuration-workflow): clock, direction, and digital enable, each as read, `ORR`, write so the other pins keep their settings. PF1-PF3 are ordinary pins, so there's nothing to unlock.
- **`DELAY`** is the countdown loop from the project's test program, as a subroutine: the count arrives in `R0`, as in [Experiment 4](/experiments/exp4-procedures-stack).
- **`SHOW`** writes a colour and waits. It calls `DELAY`, and `BL` overwrites `LR`, so `SHOW` saves its own return address with `PUSH` and returns with `POP {..., PC}`. `R4` is pushed alongside only to keep the stack 8-byte aligned, as the calling convention asks.

### Step 3: Write `__main`

Add `__main` after the subroutines:

<<< @/snippets/preview/exp5/main.asm#main

`__main` initializes Port F once, then shows the seven colours in a loop. Each colour is one `MOV` and one `BL SHOW`. `END` must stay the last line of the file.

Build, flash, and run.

<Checklist id="activity" title="Activity checklist">

- Does the LED cycle red, yellow, green, cyan, blue, magenta, white, then start again?
- With a breakpoint on the `STR` in `SHOW`, did you see `R0` take the values 0x02, 0x0A, 0x08, 0x0C, 0x04, 0x06, 0x0E?
- In **Peripherals, System Viewer, GPIOF**, did you see `DATA` change with each colour?
- Can you say why `SHOW` starts with `PUSH {R4, LR}`?

</Checklist>

::: details More to try (optional)
| Try this | What you'll see |
| --- | --- |
| Change `GPIOF_RGB` to `0x40025008`, the address masked to PF1 only | Only the red part of each colour shows: red, yellow, magenta and white light up red, the others stay dark. That's the mask at work. |
| Make one LED blink: replace the seven colours in `__main` with `RED`, then `0` (all off) | A single blinking LED, like the old Example 1. Change `DELAY_COUNT` to set the speed. |
:::

::: details The complete program
<<< @/snippets/preview/exp5/main.asm#full
:::

::: details Common problems
| Symptom | Cause and fix |
| --- | --- |
| `Undefined symbol __main` | `EXPORT __main` is missing, or the label isn't spelt `__main` (two underscores). |
| `A1163E: Unknown opcode` on an `EQU` line | The name doesn't start in column 1. Labels and `EQU` names must start at the very beginning of the line; instructions must not. |
| LED stays off | `PF_Init` isn't called, or `GPIOF_DEN` / `GPIOF_DIR` has a wrong address or bit. Check `DIR` and `DEN` read `0x0E` in the System Viewer. |
| Only one colour, never changes | `SHOW` doesn't return to `__main`: check the `PUSH {R4, LR}` and `POP {R4, PC}` pair. |
| Program stops in `HardFault_Handler` | A register was accessed before `RCGCGPIO` enabled the clock, or the stack is unbalanced (more `PUSH` than `POP`). |
| Colours change too fast or too slow | Change `DELAY_COUNT`. Each loop of `DELAY` takes about 3 clock cycles. |
:::

## Task

Turn the colour cycle into a **light show read from a table in memory**. Instead of one `MOV` and `BL` per colour, `__main` walks through a table of steps, each with a colour and its own duration, and starts again from the top when it reaches the end. Changing the show then means editing the table, not the code.

### The starter program

Start from this program in `main.s`. It assembles and runs, but the LED stays off: the setup, the subroutines and a three-step sample table are done, and the code that plays the table is yours to write.

<<< @/snippets/preview/exp5/task.asm

::: details How the provided part works
`PF_Init`, `DELAY`, and the colour names are the Activity's. `SET_LEDS` lights the colour in `R0`. `WAIT_100MS` waits about 100 ms once `WAIT_COUNT` is right. Both change only `R0` and `R1`, so `R5` and `R6` keep their values across a call: use them for the table pointer and the duration. Don't change the provided part.

The table is a list of bytes in its own `DATA` area: `DCB RED, 10` is two bytes, the colour and then 10 tenths of a second. A colour byte of `END_MARK` (`0xFF`) ends it.
:::

:::: details What to write: the three TODOs
Each TODO is highlighted in the starter, exactly where its code goes. Write them in this order: **2, 1, 3**. TODO 2 plays the sample table, TODO 1 uses that to time the steps, and TODO 3 is your own show.

::: details TODO 2: play the table (in `__main`, at `next`)
- Read the colour byte and move `R5` on: `LDRB R0, [R5], #1`. The `, #1` after the brackets is **post-indexed**, from [Experiment 2](/experiments/exp2-data-processing): load from `R5`, then add 1 to it.
- Compare `R0` with `END_MARK`. If they're equal, branch to `restart`, which points `R5` at the start of the table again.
- Read the duration the same way, into `R6`.
- Show the colour: `BL SET_LEDS` (the colour is still in `R0`).
- Wait `R6` times 100 ms: a loop that calls `WAIT_100MS`, subtracts 1 from `R6`, and repeats until it reaches 0, as in [Experiment 3](/experiments/exp3-branching-loops).
- Then `B next` for the next step. The starter's `B next` line can stay where it is.
:::

**Test now:** the LED should step red, green, blue, about one colour a second, and start again.

::: details TODO 1: time 100 ms (the `WAIT_COUNT` line, near the top)
- The sample table shows each colour for 10 tenths of a second, so one round of the three colours should take **3 seconds**.
- Time **ten** rounds with a phone stopwatch: it should be 30 seconds. With the starter's `1000000`, it's a lot less.
- Scale `WAIT_COUNT` by how far off you are: if ten rounds take 18 s, the new count is `1000000 * 30 / 18`. Rebuild, and time it again until it's within a second.
- To work it out instead: 100 ms at 50 MHz is 5,000,000 clock cycles, and one count of `DELAY` takes about 3 cycles.
:::

::: details TODO 3: your own show (in the table, at the bottom)
- Replace the three sample lines with at least **eight** steps, using at least **five** colours and at least **three** different durations. For example, a traffic light (green 3 s, yellow 1 s, red 3 s) followed by a fast white flash.
- One step per line: `DCB colour, tenths`. `OFF` is a colour too: `DCB OFF, 5` is half a second dark.
- Keep the `DCB END_MARK` line last, and don't use a duration of 0: the wait loop subtracts before it checks, so 0 wraps around to 0xFFFFFFFF and waits about 13 years.
:::
::::

Build, flash, and run.

<Checklist id="task" title="Task checklist">

- Did you time `WAIT_COUNT` yourself, so a step of `10` lasts one second against a stopwatch?
- Does your show play from the first step to the last, then start again with no pause or flicker?
- Does it use at least eight steps, five colours and three different durations?
- With a breakpoint on the `LDRB` that reads the colour, did you see `R5` go up by 2 each step and jump back to `TABLE` after the end marker?

</Checklist>

::: details What this uses
The table and the post-indexed loads come from [Experiment 2](/experiments/exp2-data-processing), the loop and the end-marker test from [Experiment 3](/experiments/exp3-branching-loops), and the subroutines from [Experiment 4](/experiments/exp4-procedures-stack).
:::
