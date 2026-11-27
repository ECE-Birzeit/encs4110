---
title: "Inter-Integrated Circuit (I2C) and Character LCD"
description: "Drive an HD44780 16x2 character LCD over two wires: configure the TM4C123's I2C module as a master, talk to the LCD's PCF8574 backpack, and show a button counter on the LCD."
outline: deep
lastUpdated: true
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=997"
short: "I2C and LCD"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Inter-Integrated Circuit (I2C) and Character LCD

This experiment drives a 16x2 character LCD, the HD44780 kind found on countless devices, over just two signal wires. A small **PCF8574 backpack** soldered to the LCD turns bytes sent over **I2C** into the levels on the LCD's pins, so the LaunchPad needs only SCL, SDA, power, and ground: four jumper wires instead of eleven. You'll learn how I2C works, set up the TM4C123's I2C module as a master, and use an LCD driver built on top of it to show text and a counter. I2C is also how most sensors, real-time clocks, and small displays connect.

::: warning New experiment: in testing
This experiment is part of the new lab manual, shared for TAs to test.
:::

## Objectives

- Describe how I2C works: two shared wires, open-drain outputs with pull-ups, addresses, START, STOP, and ACK.
- Set up the TM4C123's I2C0 module as a master and send a byte, checking that the device answered.
- Explain how the PCF8574 backpack drives an HD44780 LCD in 4-bit mode, and why each character takes six I2C bytes.
- Start the LCD and print text and numbers at a chosen row and column.
- Show a value changed by the buttons on the LCD: the interrupt handler changes it and sets a flag, `main` redraws it.

## Background

### The HD44780 LCD

The 16x2 LCD is driven by an **HD44780** controller, or a copy of it: the same commands work on almost every character LCD. The controller refreshes the screen and draws the characters by itself. The program only sends it bytes, and each byte is one of two kinds, chosen by the **RS** (register select) pin:

- **RS = 0, a command:** clear the screen, move the cursor, switch the display on, and so on.
- **RS = 1, a character:** its ASCII code is drawn at the cursor, and the cursor moves one place right.

The byte goes on the data pins, and the LCD reads it when its **E** (enable) pin falls from high to low. A third pin, **RW**, chooses read or write; it stays 0 (write) here. A **V0** pin sets the contrast.

#### 4-bit mode

The LCD has eight data pins, D0 to D7, but it can also run in **4-bit mode**, using only **D4 to D7**. Each byte then goes in two halves, called **nibbles**, upper first:

1. Put RS, and the upper nibble (bits 7 to 4) on D7 to D4. Pulse E: high, then low.
2. Put the lower nibble (bits 3 to 0) on D7 to D4. Pulse E again.

The backpack described below has only eight outputs, so it uses 4-bit mode.

#### Starting the LCD

The LCD powers up in 8-bit mode, so it has to be told to switch, with a fixed sequence:

1. Wait at least **40 ms** after power-up.
2. Send the nibble `0x3` three times: wait **over 4.1 ms** after the first, and **over 100 µs** after the other two. This puts the LCD in a known state, whatever it was doing.
3. Send the nibble `0x2`: 4-bit mode. From now on, every byte goes as two nibbles.
4. Send the commands `0x28` (4-bit, 2 lines, 5x8 dots), `0x0C` (display on, cursor off), `0x06` (cursor moves right), and `0x01` (clear).

Steps 2 and 3 send a **single** nibble each, because the LCD is still in 8-bit mode and reads only D7 to D4.

#### The commands used here

| Command | Code | What it does |
|:---|:--:|:---|
| Clear | `0x01` | Blanks the screen and moves the cursor to row 0, column 0. Takes **1.6 ms**. |
| Home | `0x02` | Moves the cursor to row 0, column 0 without clearing. Takes **1.6 ms**. |
| Entry mode | `0x06` | The cursor moves right after each character |
| Display on | `0x0C` | Display on, cursor hidden (`0x0E` shows it, `0x0F` also blinks it) |
| Function set | `0x28` | 4-bit mode, 2 lines, 5x8 dots |
| Set cursor | `0x80 + address` | Moves the cursor to a DDRAM address (below) |

Every other command takes about 40 µs.

#### Cursor positions: DDRAM addresses

The characters on the screen are kept in the LCD's **DDRAM** (display data RAM), and each position on the screen has an address. The two rows don't follow each other:

| Row | Column 0 | Column 15 | Set cursor command |
|:---|:---|:---|:---|
| 0 (top) | `0x00` | `0x0F` | `0x80 + column` |
| 1 (bottom) | `0x40` | `0x4F` | `0xC0 + column` |

So row 1, column 7 is the command `0xC0 + 7` = `0xC7`. A character stays on the screen until another one is written at its address: printing `9` where `10` was leaves the `0`, and the screen shows `90`.

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

So the LCD runs in [4-bit mode](#_4-bit-mode). The difference is how the pins change: instead of the TM4C123 driving the LCD's pins directly, the driver writes a byte to the PCF8574, and the PCF8574 drives them. Sending one nibble takes three bytes (data with E low, then E high, then E low again), and one character takes two nibbles: six I2C bytes, about **1.2 ms** per character instead of a few microseconds.

The board also has:

- A **blue trimmer potentiometer** for the contrast (the LCD's V0 pin).
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

`TPR` sets the SCL rate. Each SCL period is 10 timer periods (6 low and 4 high), and each timer period is `2 × (TPR + 1)` system clocks:

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

This experiment only uses `0x07`: every byte is its own transfer. The others send several bytes after one address, which is faster, but one byte at a time is simpler and fast enough for the LCD.

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

Drive the LCD over I2C: line 1 shows **ENCS4110 Lab** and line 2 counts up twice a second. The program has two layers: I2C functions that send one byte to a device, and LCD functions on top that send their nibbles as bytes to the backpack. Only the lowest LCD function, the one that sends a nibble, knows about I2C: the rest follow the [HD44780 steps](#the-hd44780-lcd) from the Background.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **C**, and make sure its blinking-LED test works. All the code goes in that project's `main.c`. The code assumes the 50 MHz system clock set by `SystemInit`.
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

<<< @/snippets/preview/exp11/main.c#defines{c}

- **`I2C0SCL`** and **`I2C0SDA`** are the pin bits on Port B, named after their I2C function. **`PCTL_PB23_I2C0`** puts 3 in both pins' `PCTL` fields.
- The **`MCS_`** names come in two groups because [I2CMCS](#i2cmcs-—-i2c-master-control-status) is two registers at one address: `RUN`, `START`, and `STOP` are written; `BUSY` and `ERROR` are read. `MCS_RUN` and `MCS_BUSY` are both bit 0, with different meanings.
- The **`LCD_`** bits are the [backpack's wiring](#the-pcf8574-i2c-lcd-backpack): one byte to the PCF8574 sets all eight of them at once.

### Step 3: Add the I2C functions

Add `I2C0_Init()` and `I2C0_Write()` after the definitions:

<<< @/snippets/preview/exp11/main.c#i2c{c}

- **`I2C0_Init()`** is the configuration from [The TM4C123 I2C Module](#the-tm4c123-i2c-module). Only **SDA** gets `ODR`: the datasheet says SCL must not be open drain, because the module drives it with its own pull-up. `MTPR = 24` gives 100 kHz at 50 MHz, from the [I2CMTPR formula](#i2cmtpr-—-i2c-master-timer-period).
- **`I2C0_Write()`** puts the address in bits 7:1 of `MSA` (shifted left by one; bit 0 = 0 means write) and the byte in `MDR`. Writing `STOP | START | RUN` to `MCS` makes the module do the whole [transfer](#one-transfer) by itself.
- `BUSY` can take a few cycles to rise after `MCS` is written, so the function waits for it to rise and then to fall. Waiting only for 0 could return before the transfer has even started. Then `ERROR` tells whether the device answered.

### Step 4: Add the LCD functions

Add the LCD functions after the I2C ones:

<<< @/snippets/preview/exp11/main.c#lcd{c}

- **A 1 ms clock comes first.** Once `main` starts SysTick with an interrupt every millisecond, `SysTick_Handler` counts in `ms`, and **`millis()`** returns it, like Arduino's. **`delay_ms(n)`** notes `ms` and waits until it has moved on by `n`. The subtraction `ms - start` is unsigned, so it's still right when `ms` wraps around after 49 days. SysTick keeps running for the whole program, free for anything else that needs the time.
- Never call `delay_ms()`, or any LCD function, from an interrupt handler. `SysTick_Config` gives SysTick the lowest priority, so inside another handler `ms` stops counting, and `delay_ms()` waits forever (see [Interrupt priorities](/preview/exp7-timers#interrupt-priorities) in Experiment 7).
- **`LCD_SendNibble()`** is the only LCD function that knows about I2C. It builds one byte (the nibble in bits 7:4, then `LCD_BL`, `LCD_EN`, and `RS`) and sends it three times: E low, E high, E low. The LCD latches the nibble when E falls. `LCD_BL` is in every byte because the PCF8574 shows the whole last byte on its outputs: a byte without it would switch the backlight off.
- **`LCD_Command()`**, **`LCD_Data()`**, and **`LCD_Init()`** follow the Background: two nibbles per byte, RS = 0 for commands and 1 for characters, and the [start-up sequence](#starting-the-lcd). There are almost no delays, because one nibble already takes three I2C bytes, about 0.6 ms. Only the power-up and Clear (1.6 ms) still wait.
- **`LCD_Print()`** takes a `const char *`: it only reads the string, and a string literal such as `"ENCS4110 Lab"` can't be changed.
- **`LCD_PrintNumber()`** collects the digits from the last one (`n % 10`, then `n /= 10`) and sends them in reverse, so the first digit comes out first.

### Step 5: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp11/main.c#main{c}

- `SysTick_Config(SystemCoreClock / 1000)` starts the 1 ms clock: a reload of $f_\text{clock}/1000$, with its interrupt enabled. It has to come before the first `delay_ms()`, which is in `LCD_Init()`.
- Before touching the LCD, `main` sends one byte, `LCD_BL`, to switch the backlight on. If the backpack doesn't ACK, the red LED comes on and the program stops there: that separates an I2C problem from an LCD problem.
- Then it initializes the LCD, prints both lines, and rewrites the number every 500 ms.

Build, flash, and run.

<Checklist id="activity" title="Activity checklist">

- Does the backlight come on, with no red LED?
- Does line 1 show **ENCS4110 Lab**, and line 2 **Count:** with a number going up twice a second?
- With `LCD_ADDR` changed to `0x26`, does the red LED come on? (Put it back to `0x27`.)
- Can you say how many I2C bytes one character takes, and why?
- Can you say why the code never calls an LCD function from an interrupt handler?

</Checklist>

::: details More to try (optional)
| Try this | What you'll see |
| --- | --- |
| Set `LCD_ADDR` to `0x26` | Nothing answers: the address gets a NACK, and the red LED comes on. Put it back to `0x27`. |
| Add `LCD_Command(0x0F);` after `LCD_Init();` | A blinking block cursor just after the last character printed. `0x0E` shows it without blinking. |
| Print at row 0, column 16: `LCD_SetCursor(0, 16)` | Nothing appears: column 16 is in DDRAM, but off the edge of the 16-column screen. |
| Remove `LCD_BL` from the byte in `LCD_SendNibble()` | The backlight goes off as soon as text is sent: the PCF8574 shows the whole last byte. Put it back. |
| Change `delay_ms(500)` in `main` to `delay_ms(0)` | The count races up, but only as fast as the bus allows: each redraw is a few characters, about 1.2 ms each. |
:::

::: details The complete program
<<< @/snippets/preview/exp11/main.c#full{c}
:::

::: details How slow is it?
One character is two nibbles, six I2C bytes: about **1.2 ms**. A full 16-character line takes about 20 ms, against microseconds when the LCD's pins are wired straight to the microcontroller. That's the price of using two wires instead of seven, and the reason to update only what changes on the screen.
:::

::: details Common problems
| Symptom | Cause and fix |
| --- | --- |
| Red LED on | The backpack didn't ACK: the I2C layer. Check SDA on **PB3** and SCL on **PB2** (not swapped), `VCC` on VBUS, and that `PCTL` is 3, not 7. |
| Program hangs in `I2C0_Write` | The module isn't running the bus: `RCGCI2C` or `MCR_MFE` is missing, or `AFSEL` isn't set for PB2 and PB3. |
| Backlight on, no red LED, but boxes stay on line 1 | The LCD layer: the bytes arrive, but the initialization didn't reach the LCD. Check `LCD_SendNibble()` puts the nibble in bits 7:4 and pulses `LCD_EN`. |
| Display blank, no boxes, no text | Contrast: turn the blue trimmer on the back of the backpack while the program runs. |
| Program hangs in `delay_ms()` | `ms` isn't counting: `SysTick_Config` is missing from `main`, or `delay_ms()` (or an LCD function) was called from inside an interrupt handler. |
| Random characters | A nibble went missing: the delays after the first `0x03` (5 ms) and after Clear (2 ms) are too short or missing. |
| Backlight goes off when text is printed | `LCD_BL` is missing from the byte in `LCD_SendNibble()`. |
:::

## Task

Build an **up/down counter** on the LCD. Line 1 shows `Counter`, and line 2 the count, starting at 0. **SW1** adds 1. **SW2** takes 1 away, but the count never goes below 0.

The LCD is too slow for an interrupt handler (see [Interrupt priorities](/preview/exp7-timers#interrupt-priorities) in Experiment 7), so the work is split: **the handler** changes the count and sets a flag, `changed`; **`main`** sees the flag and redraws the count.

### The starter program

Start from this program. It compiles and runs: it shows `Counter` and `0`, but the buttons don't change anything yet. The LCD stays wired as in the Activity.

<<< @/snippets/preview/exp11/task.c{c}

::: details How the provided part works
The I2C and LCD functions are the Activity's, unchanged. The buttons are set up as in the [Experiment 7 Task](/preview/exp7-timers#the-starter-program): SW1 on PF4 and SW2 on PF0 (unlocked first), each interrupting on its falling edge. Port F is set up **before** `SysTick_Config`, because `SysTick_Handler` now reads the buttons. The debounce is also the Experiment 7 Task's: each button has a `ready` flag, `GPIOF_Handler` calls `on_press()` once per press, and `SysTick_Handler` makes the button ready again 20 ms after its release. `SysTick_Handler` still counts `ms` too, for `delay_ms()`. Why not redraw in the handler? Some LCD functions call `delay_ms()`, which waits for SysTick to count `ms`; SysTick has the lowest priority, so inside `GPIOF_Handler` it can't run, and `delay_ms()` would wait forever. Don't change these.
:::

:::: details What to write: the four TODOs
Each TODO is highlighted in the starter, exactly where its code goes. Write them in this order: **3, 1, 2, 4**.

::: details TODO 3: show the count (`show_count()`)
- Move the cursor to the start of line 2 with `LCD_SetCursor(1, 0)`, as in the Activity's Step 5.
- Print enough spaces to cover the longest number (a `uint32_t` has at most 10 digits). Without them, 10 down to 9 shows `90`.
- Move the cursor back, and print `count` with `LCD_PrintNumber()`.

:::

**Test now:** start `count` at 1234 (`volatile uint32_t count = 1234;`) and re-flash: line 2 shows `1234`. Put it back to 0.

::: details TODO 1: change the count (`on_press()`)
- If `button` is `SW1`, add 1 to `count`.
- If it's `SW2`, take 1 away **only if `count` is above 0**: it's unsigned, and 0 - 1 would wrap to 4294967295.
- No LCD functions here: `on_press()` runs inside `GPIOF_Handler`.

:::

::: details TODO 2: tell `main` (`on_press()`)
- One line, after TODO 1: set `changed` to 1.

:::

::: details TODO 4: redraw (the main loop)
- If `changed` is 1: set it back to 0, then call `show_count()`.
- Clear the flag **before** redrawing: a press during the redraw sets it again, and the next time round draws the newer count.

:::
::::

Build and flash, and add `count` to the Watch window.

<Checklist id="task" title="Task checklist">

- Did you write the count change and the flag in `on_press()` yourself, with no LCD call there?
- At start, does line 1 show **Counter** and line 2 **0**?
- SW1 twelve times: **12**, one step per press, however long you hold it?
- SW2 three times: **9**, not **90**?
- SW2 until 0, then once more: does it stay at **0**?

</Checklist>
