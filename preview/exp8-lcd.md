---
title: "Character LCD over I2C"
description: "Drive an HD44780 16x2 character LCD over two wires: configure the TM4C123's I2C module as a master, talk to the LCD's PCF8574 backpack, and build an LCD driver on top."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.8"
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=997"
current: "/experiments/exp8-lcd"
short: "I2C LCD"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Character LCD over I2C

This experiment drives a 16x2 character LCD, the HD44780 kind found on countless devices, over just two signal wires. A small **PCF8574 backpack** soldered to the LCD turns bytes sent over **I2C** into the levels on the LCD's pins, so the LaunchPad needs only SCL, SDA, power, and ground: four jumper wires instead of eleven. You'll learn how the HD44780 is driven in 4-bit mode, set up the TM4C123's I2C module as a master, and build an LCD driver on top of it. I2C is also how most sensors, real-time clocks, and small displays connect, so the same bus comes back in later labs.

::: warning Reworked version: in testing
This is the reworked version of [Experiment 8](/experiments/exp8-lcd), shared for TAs to test. It moves the LCD to its I2C backpack, then has one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp8-lcd)** until this one replaces it.
:::

## Objectives

- Explain how the HD44780 LCD works: its two registers, its commands, and its DDRAM addresses.
- Start the LCD in 4-bit mode and send it commands and characters.
- Describe how I2C works: two shared wires, open-drain outputs with pull-ups, addresses, START, STOP, and ACK.
- Set up the TM4C123's I2C module as a master and send bytes, checking that the device answered.
- Drive the LCD through a PCF8574 I/O expander, and explain why it's slower than wiring the LCD's pins directly.

## Background

### HD44780 LCD Controller Architecture

The HD44780 is an LCD controller/driver developed by Hitachi and widely adopted as an industry standard for character-based LCD modules. It provides a simple microprocessor interface for controlling alphanumeric displays with minimal external components.

#### LCD Module Overview

The 16x2 LCD module consists of:

- **HD44780 Controller**: Manages display operations, character generation, and timing
- **LCD Panel**: 2 rows x 16 columns of character positions
- **Character Generator ROM (CGROM)**: Contains 208 predefined character patterns (alphanumeric, symbols, Japanese kana)
- **Character Generator RAM (CGRAM)**: 64 bytes for up to 8 user-defined custom characters (5x8 pixels each)
- **Display Data RAM (DDRAM)**: 80 bytes storing characters currently displayed (40 bytes per line)
- **Backlight**: Optional LED backlight for improved visibility

The controller handles all low-level display refresh, character rendering, and cursor management automatically. The microcontroller simply writes characters to DDRAM and issues commands for cursor positioning, display control, and special effects.

#### LCD Registers

The HD44780 has two main registers accessible by the microcontroller:

##### Instruction Register (IR)

The Instruction Register receives commands that control display operations:

- Clear display and return cursor to home
- Set cursor position in DDRAM
- Control display on/off, cursor visibility, and blinking
- Set entry mode (cursor direction, display shift)
- Shift cursor or entire display left/right
- Configure interface width (4-bit or 8-bit), number of lines, and font size

##### Data Register (DR)

The Data Register receives character codes (ASCII) to be displayed:

- Writing to DR displays a character at the current cursor position
- The cursor automatically advances after each write (direction set by entry mode)
- Reading from DR retrieves the character at the current cursor position (rarely used)

### LCD Pin Configuration

The HD44780 interface consists of 16 pins (some modules have 18 pins with additional backlight control):

| **Pin** | **Name** | **Description** |
|:--:|:---|:---|
| 1 | VSS | Ground (0V) |
| 2 | VDD | Power supply (+5V or +3.3V) |
| 3 | V0 | Contrast adjustment (connect to potentiometer) |
| 4 | RS | Register Select: 0 = Instruction (command), 1 = Data (character) |
| 5 | RW | Read/Write: 0 = Write to LCD, 1 = Read from LCD (usually grounded for write-only) |
| 6 | E | Enable: Falling edge latches data/command into LCD |
| 7­14 | D0-D7 | 8-bit data bus (D0-D3 unused in 4-bit mode) |
| 15 | A | Backlight anode (+5V, typically with series resistor) |
| 16 | K | Backlight cathode (Ground) |

*HD44780 LCD Pin Definitions*

### 4-Bit vs. 8-Bit Communication Mode

The HD44780 supports two communication modes:

#### 8-Bit Mode

- Uses all 8 data pins (D0-D7)
- Each command or character requires one write cycle
- Faster communication (single byte transfer)
- Requires 11 GPIO pins total (3 control + 8 data)

#### 4-Bit Mode

- Uses only upper 4 data pins (D4-D7), D0-D3 are left unconnected
- Each command or character requires two write cycles (upper nibble, then lower nibble)
- Slightly slower due to two transfers per byte
- Requires only 7 GPIO pins total (3 control + 4 data)
- **Preferred mode** for GPIO-constrained systems

**4-Bit Communication Protocol:**

1.  Set RS and RW to appropriate values
2.  Place upper 4 bits (bits 7-4) on D7-D4
3.  Generate enable pulse: E high → delay → E low
4.  Place lower 4 bits (bits 3-0) on D7-D4
5.  Generate another enable pulse: E high → delay → E low

In this experiment the LCD runs in 4-bit mode, but its pins aren't wired to the LaunchPad: the I2C backpack described below sets them, one byte at a time.

### LCD Initialization Sequence

Proper initialization is critical for reliable LCD operation. The HD44780 requires a specific sequence of commands with precise timing delays, especially during the transition to 4-bit mode.

#### Initialization Steps (4-Bit Mode)

The initialization sequence must account for the fact that the LCD powers up in 8-bit mode and must be switched to 4-bit mode:

1.  **Wait for power-on stabilization**: Delay at least 40 ms after VDD rises to 4.5V
2.  **Initial function set (8-bit interface)**: Send `0x30` (upper nibble only) three times:

    - First time: Wait > 4.1 ms

    - Second time: Wait > 100 μs

    - Third time: Wait > 100 μs

3.  **Switch to 4-bit mode**: Send `0x20` (upper nibble only), wait > 100 μs
4.  **Configure display parameters**: Now send full 8-bit commands in two nibbles:

    - `0x28`: Function set — 4-bit mode, 2 lines, 5x8 font

    - `0x0C`: Display control — Display ON, cursor OFF, blink OFF

    - `0x06`: Entry mode set — Increment cursor, no display shift

    - `0x01`: Clear display

    - `0x02`: Return home

**Critical Note**: Steps 2-3 send only the *upper nibble* because the LCD is still in 8-bit mode. After step 3 completes, the LCD switches to 4-bit mode, and all subsequent commands must be sent as two nibbles (upper first, then lower).

### LCD Command Set

The HD44780 supports a comprehensive set of commands for display control, cursor manipulation, and special effects.

| **Command** | **Hex Code** | **Description** |
|:---|:--:|:---|
| Clear Display | `0x01` | Clears entire display, sets DDRAM address 0, cursor to home |
| Return Home | `0x02` | Sets DDRAM address 0, cursor to home (display content unchanged) |
| Entry Mode Set | `0x04-0x07` | Controls cursor direction and display shift |
|  | `0x04` | Cursor moves left, no shift |
|  | `0x06` | Cursor moves right, no shift (default) |
|  | `0x05` | Cursor moves left, display shifts right |
|  | `0x07` | Cursor moves right, display shifts left |
| Display Control | `0x08-0x0F` | Controls display, cursor, and blink on/off |
|  | `0x08` | Display OFF |
|  | `0x0C` | Display ON, cursor OFF, blink OFF |
|  | `0x0E` | Display ON, cursor ON, blink OFF |
|  | `0x0F` | Display ON, cursor ON, blink ON |
| Cursor/Display Shift | `0x10-0x1F` | Shifts cursor or display left/right |
|  | `0x10` | Shift cursor left |
|  | `0x14` | Shift cursor right |
|  | `0x18` | Shift display left |
|  | `0x1C` | Shift display right |
| Function Set | `0x20-0x3F` | Sets interface length, line number, font |
|  | `0x28` | 4-bit mode, 2 lines, 5x8 font |
|  | `0x38` | 8-bit mode, 2 lines, 5x8 font |
| Set DDRAM Address | `0x80 + addr` | Positions cursor at DDRAM address (see [DDRAM Address Mapping](#ddram-address-mapping) for the full address layout) |

*Common HD44780 LCD Commands*

### DDRAM Address Mapping

The Display Data RAM (DDRAM) is 80 bytes, but only 32 bytes (16 per line) are visible at any time. Understanding the address mapping is essential for cursor positioning:

| **Display Position** | **DDRAM Address** | **Command** |
|:---------------------|:------------------|:------------|
| Line 1, Column 0     | `0x00`            | `0x80`      |
| Line 1, Column 1     | `0x01`            | `0x81`      |
| Line 1, Column 15    | `0x0F`            | `0x8F`      |
| Line 2, Column 0     | `0x40`            | `0xC0`      |
| Line 2, Column 1     | `0x41`            | `0xC1`      |
| Line 2, Column 15    | `0x4F`            | `0xCF`      |

*DDRAM Address Mapping for 16x2 LCD*

**Formula for cursor positioning:**
$$
\text{Command} = \begin{cases}
0x80 + \text{column} & \text{for line 1 (row 0)} \\
0xC0 + \text{column} & \text{for line 2 (row 1)}
\end{cases}
$$

### What is I2C?

I2C connects a microcontroller to many devices over just two wires:

- **SCL** (serial clock): the master's clock.
- **SDA** (serial data): the data, in both directions.

Every device on the bus connects to the same two wires and has its own **7-bit address**. The **master** (here, the TM4C123) starts every transfer and calls one device by its address; only that device answers. The others ignore the transfer.

![I2C bus: the TM4C123 and every device share SCL and SDA, pulled up to the supply by resistors; each device has its own address](/images/exp13/i2c-bus.svg)

#### Open drain and pull-ups

No device ever drives an I2C line high. Each one can only **pull the line low** or **let it go**; a **pull-up resistor** pulls a released line back high. This is called *open-drain* output, and it's what makes a shared wire safe: if two devices disagree, the line is simply low, and nothing is shorted. It's also why a TM4C123 pin must be set to open drain (`GPIOODR`) for SDA.

The PCF8574 backpack has its own pull-up resistors (usually 4.7 kΩ) to its supply, so no extra parts are needed.

#### One transfer

![One I2C write: START, the 7-bit address 0x27, the R/W bit 0, an ACK from the LCD backpack, the data byte 0x08, another ACK, and STOP](/images/exp13/i2c-frame.svg)

A write of one byte looks like this:

1. **START:** SDA falls while SCL is high. Every device starts listening.
2. **Address:** 7 bits, most significant first, one bit per SCL clock.
3. **R/W bit:** 0 = the master will write, 1 = it will read.
4. **ACK:** the master releases SDA for one clock. The device with that address pulls SDA low to say "I'm here". If no device has that address, SDA stays high: a **NACK**.
5. **Data:** 8 bits, then another ACK from the device.
6. **STOP:** SDA rises while SCL is high. The bus is free again.

Normally SDA only changes while SCL is low, and the receiver reads it while SCL is high. START and STOP are the only times SDA changes while SCL is high, which is how every device recognizes them.

| Mode | SCL rate | Used here |
|:---|:---|:---|
| Standard | 100 kHz | Yes: the PCF8574 is rated for 100 kHz |
| Fast | 400 kHz | Many sensors |
| Fast-mode Plus | 1 MHz | Some devices |

At 100 kHz each bit takes 10 µs, so one byte with its address takes about 20 clocks: **0.2 ms**.

### The PCF8574 I2C LCD backpack

The backpack is a small board soldered onto the LCD's 16 pins. Its main chip, the **PCF8574**, is an *I/O expander*: it has 8 output pins, P0 to P7, and whatever byte you write to it over I2C appears on them. On the backpack, those 8 pins are wired to the LCD:

| PCF8574 pin | Bit | LCD signal |
|:---|:---|:---|
| P7 to P4 | 7 to 4 | D7 to D4 |
| P3 | 3 | Backlight (1 = on) |
| P2 | 2 | E (enable) |
| P1 | 1 | RW (always 0 here) |
| P0 | 0 | RS (0 = command, 1 = character) |

So the LCD runs in **4-bit mode**, as described in [4-Bit Mode](#_4-bit-mode) above. The difference is how the pins change: instead of writing `GPIOB->DATA`, the driver writes a byte to the PCF8574. Sending one nibble takes three bytes (data with E low, then E high, then E low again), and one character takes two nibbles: six I2C bytes, about **1.2 ms** per character instead of a few microseconds.

The board also has:

- A **blue trimmer potentiometer** for the contrast (the V0 pin in the [pin table](#lcd-pin-configuration) above).
- A **jumper** for the backlight.
- Three solder pads, **A0 to A2**, that change the address. Unsoldered, the address is **0x27** for a PCF8574T chip, which is what the lab's boards use. (Boards with a PCF8574AT chip are at **0x3F**.)

| Backpack pin | Connect to |
|:---|:---|
| GND | **GND** |
| VCC | **VBUS** (5 V: the LCD needs it) |
| SDA | **PB3** (I2C0SDA) |
| SCL | **PB2** (I2C0SCL) |

::: info Isn't 5 V too much for the TM4C123?
The backpack's pull-ups go to its own VCC, so SDA and SCL are pulled up to 5 V. PB2 and PB3 are 5 V tolerant (only PB0, PB1, PD4, and PD5 aren't), and in open drain the TM4C123 only ever pulls the lines low, so this is the usual way to connect it.
:::

### The TM4C123 I2C Module

The TM4C123GH6PM has four I2C modules, each able to act as a master or a slave. This experiment uses **I2C0** as a master on its default pins:

| Module | SCL | SDA | PCTL value |
|:---|:---|:---|:---|
| **I2C0** | **PB2** | **PB3** | 3 |
| I2C1 | PA6 | PA7 | 3 |
| I2C2 | PE4 | PE5 | 3 |
| I2C3 | PD0 | PD1 | 3 |

::: warning Avoid I2C3 on the LaunchPad
On the LaunchPad, PD0 and PD1 are connected to PB6 and PB7 through two 0 Ω resistors (R9 and R10), so I2C3 can't be used unless those pins are left alone or the resistors are removed. I2C0 has no such problem.
:::

As a master, the module handles the whole transfer: you write the address, the data, and a command, and it produces START, the clock, the bits, checks the ACKs, and STOP.

### I2C Registers

The I2C0 registers are reached as `I2C0->MSA`, `I2C0->MCS`, and so on (base `0x40020000`). Only the master registers are used here. First, the clock:

#### RCGCI2C — I2C Run Mode Clock Gating

**Register:** `SYSCTL->RCGCI2C` (`0x400FE620`)

Bit *n* enables I2C*n*: bit 0 for I2C0. `SYSCTL->PRI2C` has the matching "ready" bits; the driver waits on it instead of a fixed delay, the same way it waits on `PRGPIO` for Port B.

#### GPIOODR — Open Drain Select

**Register:** `GPIOx->ODR` (`Base + 0x50C`)

Setting bit *n* makes pin *n* open drain. Set it for **SDA only** (PB3). The datasheet says SCL must *not* be open drain: the I2C module drives it with its own active pull-up.

#### I2CMCR — I2C Master Configuration

**Register:** `I2Cx->MCR` (`Base + 0x020`)

**I2CMCR Register — I2C Master Configuration**

| Bits | Field |
| --- | --- |
| 31–7 | reserved |
| 6 | GFE |
| 5 | SFE |
| 4 | MFE |
| 3–1 | reserved |
| 0 | LPBK |

- **Bit 4 (MFE):** master function enable. Write `0x10` to make the module a master.
- **Bit 5 (SFE):** slave function enable; **bit 6 (GFE):** glitch filter; **bit 0 (LPBK):** internal loopback for testing. All 0 here.

#### I2CMTPR — I2C Master Timer Period

**Register:** `I2Cx->MTPR` (`Base + 0x00C`)

**I2CMTPR Register — I2C Master Timer Period**

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7 | HS |
| 6–0 | TPR |

`TPR` sets the SCL rate. Each SCL period is 10 timer periods (6 low and 4 high), and each timer period is `TPR + 1` system clocks:

$$
\text{TPR} = \frac{f_{\text{clock}}}{2 \times (6 + 4) \times f_{\text{SCL}}} - 1
$$

At 50 MHz and 100 kHz: $\text{TPR} = 50{,}000{,}000 / 2{,}000{,}000 - 1 = 24$. For 400 kHz it would be about 5.25, so 5 (a bit faster than 400 kHz) or 6 (a bit slower). Leave `HS` (high-speed mode) at 0.

#### I2CMSA — I2C Master Slave Address

**Register:** `I2Cx->MSA` (`Base + 0x000`)

**I2CMSA Register — I2C Master Slave Address**

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7–1 | SA |
| 0 | R/S |

The 7-bit address goes in bits 7:1, so it's **shifted left by one**, and bit 0 chooses the direction: 0 to send (write), 1 to receive (read). For the backpack at 0x27: `I2C0->MSA = 0x27 << 1` = `0x4E`.

#### I2CMDR — I2C Master Data

**Register:** `I2Cx->MDR` (`Base + 0x008`)

**I2CMDR Register — I2C Master Data**

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7–0 | DATA |

The byte to send, written before starting the transfer (or the byte received, after a read).

#### I2CMCS — I2C Master Control/Status

**Register:** `I2Cx->MCS` (`Base + 0x004`)

This register is two registers at one address: **writing** it gives the module a command, and **reading** it returns the status.

**I2CMCS Register — written (control)**

| Bits | Field |
| --- | --- |
| 31–5 | reserved |
| 4 | HS |
| 3 | ACK |
| 2 | STOP |
| 1 | START |
| 0 | RUN |

| Value written | Meaning |
|:---|:---|
| `0x07` (STOP, START, RUN) | A complete transfer of one byte: START, address, data, STOP |
| `0x03` (START, RUN) | START, address, first byte, and keep the bus for more bytes |
| `0x01` (RUN) | Send the next byte in `MDR`, keep the bus |
| `0x05` (STOP, RUN) | Send the last byte, then STOP |

This experiment only uses `0x07`: every byte is its own transfer. The others send several bytes after one address, which is faster (see the [Task](#task)'s last hint).

**I2CMCS Register — read (status)**

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7 | CLKTO |
| 6 | BUSBSY |
| 5 | IDLE |
| 4 | ARBLST |
| 3 | DATACK |
| 2 | ADRACK |
| 1 | ERROR |
| 0 | BUSY |

| Bit | Name | 1 means |
|:---|:---|:---|
| 0 | BUSY | The module is in the middle of a transfer. While it's 1, the other bits aren't valid. |
| 1 | ERROR | The last transfer failed: the address or the data got no ACK |
| 2 | ADRACK | The address got no ACK: no device has that address |
| 3 | DATACK | The data byte got no ACK |
| 4 | ARBLST | Another master won the bus |
| 6 | BUSBSY | The bus is in use (between a START and a STOP) |

## Activity

Drive the LCD over I2C: line 1 shows **ENCS4110 Lab** and line 2 counts up twice a second. The program has two layers: I2C functions that send one byte to a device, and LCD functions on top that send their nibbles as bytes to the backpack. Only the lowest LCD function, the one that sends a nibble, knows about I2C: the rest follow the HD44780 steps from the Background.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, and make sure its blinking-LED test works. All the code goes in that project's `main.c`. The code assumes the 50 MHz system clock set by `SystemInit`.
:::

### Step 1: Wire the LCD

Connect the backpack's four pins to the LaunchPad:

| Backpack | LaunchPad | Signal |
| --- | --- | --- |
| `GND` | **GND** | |
| `VCC` | **VBUS** | 5 V for the LCD and the pull-ups |
| `SDA` | **PB3** | I2C0SDA |
| `SCL` | **PB2** | I2C0SCL |

The backlight should light up as soon as the LaunchPad has power, and line 1 shows a row of solid boxes: the LCD has power but hasn't been initialized yet. No boxes at all? Turn the blue contrast trimmer on the back of the backpack.

### Step 2: Add the definitions

Replace the contents of `main.c` with the include and the definitions:

<<< @/snippets/preview/exp8/main.c#defines{c}

- **`I2C0SCL`** and **`I2C0SDA`** are the pin bits on Port B, named after their I2C function. **`PCTL_PB23_I2C0`** puts 3 in both pins' `PCTL` fields.
- The **`MCS_`** names come in two groups because [I2CMCS](#i2cmcs-—-i2c-master-control-status) is two registers at one address: `RUN`, `START`, and `STOP` are written; `BUSY` and `ERROR` are read. `MCS_RUN` and `MCS_BUSY` are both bit 0, with different meanings.
- The **`LCD_`** bits are the [backpack's wiring](#the-pcf8574-i2c-lcd-backpack): one byte to the PCF8574 sets all eight of them at once.

### Step 3: Add the I2C functions

Add `I2C0_Init()` and `I2C0_Write()` after the definitions:

<<< @/snippets/preview/exp8/main.c#i2c{c}

- **`I2C0_Init()`** is the configuration from [The TM4C123 I2C Module](#the-tm4c123-i2c-module). Only **SDA** gets `ODR`: the datasheet says SCL must not be open drain, because the module drives it with its own pull-up. `MTPR = 24` gives 100 kHz at 50 MHz, from the [I2CMTPR formula](#i2cmtpr-—-i2c-master-timer-period).
- **`I2C0_Write()`** puts the address in bits 7:1 of `MSA` (shifted left by one; bit 0 = 0 means write) and the byte in `MDR`. Writing `STOP | START | RUN` to `MCS` makes the module do the whole [transfer](#one-transfer) by itself.
- `BUSY` can take a few cycles to rise after `MCS` is written, so the function waits for it to rise and then to fall. Waiting only for 0 could return before the transfer has even started. Then `ERROR` tells whether the device answered.

### Step 4: Add the LCD functions

Add the LCD functions after the I2C ones:

<<< @/snippets/preview/exp8/main.c#lcd{c}

- **A 1 ms clock comes first.** Once `main` starts SysTick with an interrupt every millisecond, `SysTick_Handler` counts in `ms`, and **`millis()`** returns it, like Arduino's. **`delay_ms(n)`** notes `ms` and waits until it has moved on by `n`. The subtraction `ms - start` is unsigned, so it's still right when `ms` wraps around after 49 days. SysTick keeps running for the whole program, free for anything else that needs the time.
- Never call `delay_ms()`, or any LCD function, from an interrupt handler. SysTick has the lowest priority, so inside another handler `ms` stops counting, and `delay_ms()` waits forever.
- **`LCD_SendNibble()`** is the only LCD function that knows about I2C. It builds one byte (the nibble in bits 7:4, then `LCD_BL`, `LCD_EN`, and `RS`) and sends it three times: E low, E high, E low. The LCD latches the nibble when E falls. `LCD_BL` is in every byte because the PCF8574 shows the whole last byte on its outputs: a byte without it would switch the backlight off.
- **`LCD_Command()`**, **`LCD_Data()`**, and **`LCD_Init()`** follow the Background: two nibbles per byte, RS = 0 for commands and 1 for characters, and the [initialization sequence](#lcd-initialization-sequence). There are almost no delays, because one nibble already takes three I2C bytes, about 0.6 ms. Only the power-up and Clear (1.6 ms) still wait.
- **`LCD_Print()`** takes a `const char *`: it only reads the string, and a string literal such as `"ENCS4110 Lab"` can't be changed.
- **`LCD_PrintNumber()`** collects the digits from the last one (`n % 10`, then `n /= 10`) and sends them in reverse, so the first digit comes out first.

### Step 5: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp8/main.c#main{c}

- `SysTick_Config(SystemCoreClock / 1000)` starts the 1 ms clock: a reload of $f_\text{clock}/1000$, with its interrupt enabled. It has to come before the first `delay_ms()`, which is in `LCD_Init()`.
- Before touching the LCD, `main` sends one byte, `LCD_BL`, to switch the backlight on. If the backpack doesn't ACK, the red LED comes on and the program stops there: that separates an I2C problem from an LCD problem.
- Then it initializes the LCD, prints both lines, and rewrites the number every 500 ms.

**Check:** build, flash, and run. Line 1 shows **ENCS4110 Lab** and line 2 **Count:** followed by a number that goes up twice a second. If something's wrong, the board tells you which layer:

- **Red LED on:** the I2C layer. The backpack didn't answer at `LCD_ADDR`.
- **No red LED, but the boxes stay on line 1:** the LCD layer. The bytes arrive, but the initialization didn't work.

**Then try** what the ACK means: change `LCD_ADDR` to `0x26` and re-flash. Now nothing answers, the address gets a NACK, and the red LED comes on. Put it back to `0x27`.

::: details The complete program
<<< @/snippets/preview/exp8/main.c#full{c}
:::

::: info How slow is it?
One character is two nibbles, six I2C bytes: about **1.2 ms**. A full 16-character line takes about 20 ms, against microseconds when the LCD's pins are wired straight to the microcontroller. That's the price of using two wires instead of seven, and the reason to update only what changes on the screen.
:::

## Common problems

| Symptom | Cause and fix |
| --- | --- |
| Red LED on | The backpack didn't ACK. Check SDA on **PB3** and SCL on **PB2** (not swapped), `VCC` on VBUS, and that `PCTL` is 3, not 7. |
| Program hangs in `I2C0_Write` | The module isn't running the bus: `RCGCI2C` or `MCR_MFE` is missing, or `AFSEL` isn't set for PB2 and PB3. |
| Backlight on, but boxes stay on line 1 | The initialization didn't reach the LCD. Check `LCD_SendNibble()` puts the nibble in bits 7:4 and pulses `LCD_EN`. |
| Display blank, no boxes, no text | Contrast: turn the blue trimmer on the back of the backpack while the program runs. |
| Program hangs in `delay_ms()` | `ms` isn't counting: `SysTick_Config` is missing from `main`, or `delay_ms()` was called from inside an interrupt handler. |
| Random characters | A nibble went missing: the delays after the first `0x03` (5 ms) and after Clear (2 ms) are too short or missing. |
| Backlight goes off when text is printed | `LCD_BL` is missing from the byte in `LCD_SendNibble()`. |

## Task

Build a **stopwatch** on the LCD. Line 1 shows a title, and line 2 the time as `mm:ss.t` (minutes, seconds, tenths). **SW1** starts and stops it, and the green LED is on while it runs. **SW2** resets it to `00:00.0`, but only while it's stopped.

![State diagram of the stopwatch. Stopped, LED off: SW1 starts it, noting the time with millis(), and goes to Running; SW2 resets the time to 00:00.0 and stays in Stopped. Running, green: SW1 stops it, adds the run to the total, and goes back to Stopped; SW2 is ignored.](/images/exp8/stopwatch.svg)

SW2 has no arrow out of Running: while the stopwatch runs, it does nothing.

### The starter program

The LCD is wired as in [Step 1](#step-1-wire-the-lcd). The starter is three files. `lcd.c` and `lcd.h` are the I2C and LCD driver you wrote in the Activity, moved into a file of their own. Add both to your project: in Keil's **Project** window, right-click **Source Group 1**, choose **Add Existing Files to Group**, and pick `lcd.c` (keep `lcd.h` in the same folder). `main.c` is the stopwatch, with three TODOs left for you. This time the TODOs are bigger, and the hints say what to do rather than how.

::: code-group
<<< @/snippets/preview/exp8/task.c{c} [main.c]
<<< @/snippets/preview/exp8/lcd.h{c} [lcd.h]
<<< @/snippets/preview/exp8/lcd.c{c} [lcd.c]
:::

**The provided part.** The debouncing is the simplest kind: act on the press, then wait for the release. A falling edge on SW1 or SW2 calls `GPIOF_Handler`, which calls `on_press()` once and turns the buttons' interrupts off, so the bounces after it are ignored. `SysTick_Handler`, besides counting `ms` for `millis()`, checks the buttons every millisecond. Once both have been up for `DEBOUNCE_MS` (20 ms) in a row, it clears the release bounces and turns the interrupts back on. That's why `lcd.c` has no `SysTick_Handler` of its own, and why Port F is set up before `SysTick_Config`. `main` then starts the 1 ms clock and calls `I2C0_Init()` and `LCD_Init()`, as in the Activity. The main loop redraws the time only when the tenth on the screen changes, so the LCD isn't rewritten a thousand times a second. Don't change these.

:::: details What to write: the three TODOs
Each TODO is highlighted in `main.c`. Write them in order: **1, 2, 3**. The stopwatch needs no timer of its own: it measures each run against `millis()`, the Activity's 1 ms clock.

::: details TODO 1: the buttons (on_press)
- Follow the arrows in the diagram.
- SW1 while stopped starts a run: note the time in `start_ms` with `millis()`, set `running`, and turn the green LED on.
- SW1 while running ends it: add `millis() - start_ms` to `total_ms`, clear `running`, and turn the LED off.
- SW2 sets `total_ms` back to 0, but only while the stopwatch is stopped. The screen follows by itself: the main loop sees the time change and redraws `00:00.0`.
- `on_press()` runs **inside the button interrupt**, so it only changes variables and the LED. Never call an LCD function from it: the main loop may be halfway through writing the screen, and the interrupt's I2C bytes would land in the middle, so the LCD shows garbage.
- Change only the green LED's bit (`|=` and `&= ~`), not the whole `GPIOF->DATA`.
:::

::: details TODO 2: the time (elapsed)
- Return `total_ms`, plus the current run, `millis() - start_ms`, while it's running.
- `on_press()` runs inside the button interrupt, so it can change the three variables at any moment, even halfway through `elapsed()` reading them. If SW1 stopped the stopwatch between your read of `running` and your read of `total_ms`, you'd count the last run twice, and the wrong time would stay frozen on the screen.
- So read them with interrupts off: `__disable_irq()` before the first read, `__enable_irq()` after the last. Keep that stretch short: a few lines, no LCD calls.
:::

**Test now:** add `running`, `total_ms`, and `elapsed()`'s result to the Watch window (or watch `total_ms` after each stop). SW1 turns the green LED on and off, `total_ms` grows by the length of each run, and SW2 zeroes it only while the LED is off. The LCD still shows only the title.

::: details TODO 3: the display (show_time)
- Split `t` (milliseconds) into minutes, seconds, and tenths of a second. Minutes go past 59, so keep two digits with `% 100`.
- Print them on line 2 from column 0 as `mm:ss.t`, with every field padded to its width: `01:05.3`, never `1:5.3`. Print one character at a time with `LCD_Data()`. The character for a digit `d` is `'0' + d`.
:::
::::

**Check:** build and flash.

- At start, line 1 shows **Stopwatch** and line 2 **00:00.0**.
- SW1: the time runs and the green LED is on. SW1 again: it stops and holds.
- SW2 while stopped: back to **00:00.0**. SW2 while running: nothing happens.
- Let it run past a minute: **00:59.9** becomes **01:00.0**. Against a phone stopwatch, it keeps time to within a tenth over a minute.
