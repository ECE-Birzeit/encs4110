---
title: "Character LCD Display Interface"
description: "This experiment introduces LCD interfacing using the HD44780-based 16x2 character display module."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.8"
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=649"
current: "/experiments/exp8-lcd"
short: "Parallel LCD"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Character LCD Display Interface

This experiment introduces LCD interfacing using the HD44780-based 16x2 character display module. You will learn the controller’s architecture, implement 4-bit communication mode to minimize GPIO usage, and create a reusable LCD driver library with functions for initialization, command transmission, and data display. By the end of this lab, you will be able to interface an LCD with the TM4C123 microcontroller and create interactive display applications combining LCD output with GPIO input and interrupts.

::: warning Preview: new format
This is the reworked version of [Experiment 8](/experiments/exp8-lcd), shared for TAs to test. It has the same background, then one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp8-lcd)** until this one replaces it.
:::


## Objectives

- Understand the architecture and operation of the HD44780-based 16x2 LCD module.
- Differentiate between command and data registers and their respective control signals.
- Configure the LCD in 4-bit mode to minimize GPIO pin usage.
- Implement the initialization sequence for 4-bit LCD operation.
- Send commands and data to the LCD using proper timing and enable pulse sequences.
- Display text strings at specific cursor positions on the LCD.
- Implement scrolling and dynamic display effects using LCD commands.
- Integrate LCD interfacing with GPIO and interrupt-driven input from push buttons.

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

In this experiment, we use 4-bit mode with the following connections:

- **PB0** → RS (Register Select)
- **PB2** → E (Enable)
- **PB4-PB7** → D4-D7 (Data pins)
- **PB1** → RW (the code holds it low: write only)

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

### Timing Requirements

The HD44780 datasheet specifies minimum pulse widths and setup/hold times (e.g., enable pulse width ≥450 ns, enable cycle ≥1 μs, RS/data setup and hold times of tens of nanoseconds) that are far shorter than anything achievable with a simple software delay loop — in practice, the driver below uses generously-rounded, safe delays instead of hitting those minimums precisely:

- **Around each enable pulse** (`LCD_EnablePulse()`): a ~1 μs `delay_us()` before and after toggling E, comfortably covering the enable pulse width/cycle and RS/data setup-hold requirements together.
- **After most commands**: ~1-2 ms (datasheet minimum is ~40 μs).
- **After `Clear Display` (`0x01`) or `Return Home` (`0x02`)**: ~2 ms (datasheet minimum is ~1.6 ms), since these two commands take noticeably longer than other commands.

We implement these delays using the SysTick timer (at 50 MHz, 1 μs = 50 clock cycles) for precise, non-blocking-free timing.

### LCD Driver Implementation Strategy

A well-structured LCD driver separates concerns into distinct layers:

##### Low-Level Functions

- `LCD_EnablePulse()`: Generates enable pulse (E high → delay → E low)
- `LCD_SendNibble()`: Sends 4 bits on D7-D4 with enable pulse
- `delay_us()`, `delay_ms()`: Precise timing using SysTick

##### Mid-Level Functions

- `LCD_Command()`: Sends 8-bit command (RS=0) as two nibbles
- `LCD_Data()`: Sends 8-bit character (RS=1) as two nibbles

##### High-Level API

- `LCD_Init()`: Complete initialization sequence
- `LCD_Clear()`: Clears display
- `LCD_SetCursor(row, col)`: Positions cursor
- `LCD_Print(string)`: Displays string at current cursor position

## Activity

Drive a 16x2 character LCD from seven GPIO pins: line 1 shows **ENCS4110 Lab** and line 2 counts up twice a second. You write the driver from the bottom up: first the pins and the enable pulse, then commands and characters, then text and numbers. The same function names come back in [Experiment 13](/experiments/exp13-i2c-lcd), where only the lowest layer changes.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **C**, and make sure its blinking-LED test works. All the code goes in that project's `main.c`.
:::

### Step 1: Wire the LCD

![Wiring the 16x2 LCD: a 5 V rail from VBUS and a GND rail; VSS and K to GND, VDD and A to 5 V; a 10 kilo-ohm contrast potentiometer with its outer pins on 5 V and GND and its middle pin to V0; RS, RW and E to PB0, PB1 and PB2; D4 to D7 to PB4 to PB7; D0 to D3 not connected](/images/exp8/lcd-wiring.svg)

| LCD pin | Connect to | Why |
| --- | --- | --- |
| VSS (1), K (16) | **GND** | Ground and the backlight's cathode |
| VDD (2), A (15) | **VBUS** (5 V) | Supply and the backlight's anode |
| V0 (3) | Wiper of a 10 kΩ potentiometer between 5 V and GND | Contrast |
| RS (4) | **PB0** | Command or character |
| RW (5) | **PB1** | Write only: the code holds it low (RW straight to GND works too) |
| E (6) | **PB2** | Enable |
| D4 to D7 (11 to 14) | **PB4 to PB7** | Data, 4 bits at a time |

D0 to D3 stay unconnected. With power on and no program running, turn the contrast potentiometer until line 1 shows a row of solid boxes: the LCD works, but hasn't been initialized yet.

### Step 2: Add the definitions

Replace the contents of `main.c` with the include and the definitions:

<<< @/snippets/preview/exp8/main.c#defines{c}

- The pin names follow the wiring. **`LCD_DATA`** is the four data pins together, `0xFU << 4`, so one masked write sets D4 to D7 at once.
- The **`CMD_`** names are the [HD44780 commands](#lcd-command-set) the program uses. `CMD_LINE1` and `CMD_LINE2` are "set DDRAM address" with the address of each line's first column, from [DDRAM Address Mapping](#ddram-address-mapping).

### Step 3: Add the low-level functions

Add the delays and `LCD_SendNibble()` after the definitions:

<<< @/snippets/preview/exp8/main.c#low{c}

- **`delay_us()`** uses SysTick by polling: load a count, wait for `COUNTFLAG`, stop. No interrupt, so SysTick is busy only while a delay runs.
- **`LCD_SendNibble()`** is the heart of [4-bit mode](#_4-bit-vs-8-bit-communication-mode): it puts four bits on D7-D4, then pulses E. The LCD reads the data when E **falls**. The 1 µs waits are far longer than the LCD's minimums (a few hundred nanoseconds), which keeps the timing safe without measuring it.
- The `DATA` write is masked with `~LCD_DATA`, so RS, RW, and E keep their levels while the data pins change.

### Step 4: Add the LCD functions

Add the command, character, and text functions after `LCD_SendNibble()`:

<<< @/snippets/preview/exp8/main.c#lcd{c}

- **`LCD_Command()`** and **`LCD_Data()`** send a byte as two nibbles, upper first. The only difference is RS: 0 for a command, 1 for a character. Each then waits for the LCD to finish: about 40 µs, except Clear, which takes 1.6 ms.
- **`LCD_Init()`** configures the pins and then follows the [initialization sequence](#initialization-steps-4-bit-mode). The three `0x3` nibbles bring the LCD to a known 8-bit state whatever it was doing; `0x2` switches it to 4-bit mode; from then on every byte goes as two nibbles.
- **`LCD_SetCursor()`** adds the column to the line's start address. **`LCD_Print()`** sends characters until the string's terminating 0. **`LCD_PrintNumber()`** collects the digits last first (`n % 10`, then `n /= 10`) and sends them in reverse.

### Step 5: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp8/main.c#main{c}

**Check:** build, flash, and run. The boxes disappear, line 1 shows **ENCS4110 Lab**, and line 2 **Count:** with a number going up twice a second.

**Then try:**

- Comment out the `delay_us(1)` after `GPIOB->DATA |= LCD_EN;` and re-flash. It probably still works: the LCD needs E high for only 450 ns, and the instructions around it take a few tens of nanoseconds each. Now also remove the 5 ms delay after the first `0x3`: the LCD usually fails to start, because that one wait is a real requirement.
- Print `"ENCS4110 Laboratory!"` (20 characters) on line 1. The last four characters don't appear on line 2: DDRAM line 1 runs to address `0x27`, so they're stored off-screen. `LCD_SetCursor()` is how you move to line 2.

::: details The complete program
<<< @/snippets/preview/exp8/main.c#full{c}
:::

## Common problems

| Symptom | Cause and fix |
| --- | --- |
| Nothing at all, no boxes | Contrast: turn the potentiometer on V0. If it still shows nothing, check VDD on 5 V and VSS on GND. |
| Boxes on line 1 stay after the program starts | The initialization didn't reach the LCD: check E on **PB2**, RS on **PB0**, and D4-D7 in order on PB4-PB7. Swapped data wires are the usual cause. |
| Random characters | A nibble is lost: the delays in `LCD_Init()` are too short, or RW isn't held low (PB1 low, or RW on GND). |
| Text appears, then the screen goes blank | A `CMD_CLEAR` with no 2 ms wait after it, so the next command arrives while the LCD is still clearing. |
| Numbers leave old digits behind (`109` after `99`) | Not a problem when the number only grows; a number that can shrink needs spaces printed after it. |

## Task

Build a **stopwatch**. Line 1 shows a title, and line 2 the time as `mm:ss.t` (minutes, seconds, tenths). **SW1** starts and stops it; **SW2** resets it to `00:00.0`, but only while it's stopped.

**Requirements:**
- Count time with **Timer1A** in periodic mode, interrupting every **1 ms**, as in [Experiment 7](/preview/exp7-timers). Its handler keeps a millisecond clock, `ms`, and adds to the stopwatch's time while it runs. SysTick can't be the clock this time: the LCD driver uses it for its delays (see the hint).
- Debounce both buttons with the quiet-time rule from [Experiment 7](/preview/exp7-timers), using Timer1A's `ms`. SW2 is on PF0: unlock it as in [Experiment 6](/preview/exp6-gpio-interrupts).
- Update the LCD from the **main loop**, never from an interrupt handler, and only when the shown tenth has changed.
- Always print two digits for minutes and seconds (`01:05.3`, not `1:5.3`).

::: details Hint: why not SysTick?
`delay_us()` reprograms SysTick every time it runs: it overwrites `LOAD`, clears `VAL`, and turns SysTick off at the end. A SysTick interrupt clock would stop every time the LCD is written. One timer can't do two jobs: give the delays SysTick and the clock Timer1A.
:::

::: details Hint: updating the screen
Writing a character takes about 50 µs, so the time field (7 characters) takes under half a millisecond: fine for the main loop, but far too long for a handler that runs every millisecond. Keep the running time in a `volatile uint32_t`, and in the main loop compare its tenths with the last value you printed. For two digits, print `n / 10` and `n % 10` as characters: `'0' + digit`.
:::
