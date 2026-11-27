---
title: "I2C with a 16x2 LCD"
short: "I2C LCD"
description: "Talk to devices over two wires with the TM4C123's I2C module: check a device answers at its address, then drive a 16x2 character LCD through a PCF8574 I2C backpack."
outline: deep
lastUpdated: true
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=997"
preview: "/preview/exp11-i2c-lcd"
part: "Part 3: Advanced Peripherals"
---

# I2C with a 16x2 LCD

In [Experiment 8](./exp8-lcd) the 16x2 LCD took seven GPIO pins. This experiment drives the same display with **two**, using **I2C** (Inter-Integrated Circuit), the bus that most sensors, real-time clocks, EEPROMs, and small displays use. You will configure the TM4C123's I2C module as a master, check that the LCD answers at its address, and then rebuild the Experiment 8 LCD driver for an LCD with a **PCF8574 I2C backpack** soldered on, keeping the same functions.

::: warning In development
This experiment is new and isn't in the lab manual yet. If something on this page doesn't match what you see on the board, tell your instructor.
:::

## Objectives

- Describe how I2C works: two shared wires, open-drain outputs with pull-ups, addresses, START, STOP, and ACK.
- Configure an I2C module as a master: pins, open drain, `I2CMCR`, and the clock rate in `I2CMTPR`.
- Send a byte with `I2CMSA`, `I2CMDR`, and `I2CMCS`, and check for errors.
- Tell from the ACK whether a device is present at an address.
- Drive an HD44780 LCD through a PCF8574 I/O expander, and understand why it's slower than the parallel version.

## Background

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

So the LCD still runs in **4-bit mode**, exactly as in Experiment 8. The difference is how the pins change: instead of writing `GPIOB->DATA`, the driver writes a byte to the PCF8574. Sending one nibble takes three bytes (data with E low, then E high, then E low again), and one character takes two nibbles: six I2C bytes, about **1.2 ms** per character instead of a few microseconds.

The board also has:

- A **blue trimmer potentiometer** for the contrast (the V0 pin from Experiment 8).
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

Bit *n* enables I2C*n*: bit 0 for I2C0. `SYSCTL->PRI2C` has the matching "ready" bits; the driver waits on it instead of a fixed delay, as the Experiment 8 driver does with `PRGPIO`.

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

Drive the LCD over I2C: line 1 shows **ENCS4110 Lab** and line 2 counts up twice a second. The program has two layers: I2C functions that send one byte to a device, and LCD functions on top that send their nibbles as bytes to the backpack. The LCD functions have the same names as in Experiment 8, so only the lowest one is really new.

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

<<< @/snippets/i2c/main.c#defines{c}

- **`I2C0SCL`** and **`I2C0SDA`** are the pin bits on Port B, named after their I2C function. **`PCTL_PB23_I2C0`** puts 3 in both pins' `PCTL` fields.
- The **`MCS_`** names come in two groups because [I2CMCS](#i2cmcs-—-i2c-master-control-status) is two registers at one address: `RUN`, `START`, and `STOP` are written; `BUSY` and `ERROR` are read. `MCS_RUN` and `MCS_BUSY` are both bit 0, with different meanings.
- The **`LCD_`** bits are the [backpack's wiring](#the-pcf8574-i2c-lcd-backpack): one byte to the PCF8574 sets all eight of them at once.

### Step 3: Add the I2C functions

Add `I2C0_Init()` and `I2C0_Write()` after the definitions:

<<< @/snippets/i2c/main.c#i2c{c}

- **`I2C0_Init()`** is the configuration from [The TM4C123 I2C Module](#the-tm4c123-i2c-module). Only **SDA** gets `ODR`: the datasheet says SCL must not be open drain, because the module drives it with its own pull-up. `MTPR = 24` gives 100 kHz at 50 MHz, from the [I2CMTPR formula](#i2cmtpr-—-i2c-master-timer-period).
- **`I2C0_Write()`** puts the address in bits 7:1 of `MSA` (shifted left by one; bit 0 = 0 means write) and the byte in `MDR`. Writing `STOP | START | RUN` to `MCS` makes the module do the whole [transfer](#one-transfer) by itself.
- `BUSY` can take a few cycles to rise after `MCS` is written, so the function waits for it to rise and then to fall. Waiting only for 0 could return before the transfer has even started. Then `ERROR` tells whether the device answered.

### Step 4: Add the LCD functions

Add the LCD functions after the I2C ones:

<<< @/snippets/i2c/main.c#lcd{c}

- **`LCD_SendNibble()`** is the only function that changes from Experiment 8. Instead of writing `GPIOB->DATA`, it builds one byte (the nibble in bits 7:4, then `LCD_BL`, `LCD_EN`, and `RS`) and sends it three times: E low, E high, E low. The LCD latches the nibble when E falls. `LCD_BL` is in every byte because the PCF8574 shows the whole last byte on its outputs: a byte without it would switch the backlight off.
- **`LCD_Command()`**, **`LCD_Data()`**, and **`LCD_Init()`** are the Experiment 8 functions: two nibbles per byte, RS = 0 for commands and 1 for characters, and the same start-up sequence. There are almost no delays, because one nibble already takes three I2C bytes, about 0.6 ms. Only the power-up and Clear (1.6 ms) still wait.
- **`LCD_Print()`** takes a `const char *`: it only reads the string, and a string literal such as `"ENCS4110 Lab"` can't be changed.
- **`LCD_PrintNumber()`** collects the digits from the last one (`n % 10`, then `n /= 10`) and sends them in reverse, so the first digit comes out first.

### Step 5: Write `main`

Add `main` at the end of the file:

<<< @/snippets/i2c/main.c#main{c}

- Before touching the LCD, `main` sends one byte, `LCD_BL`, to switch the backlight on. If the backpack doesn't ACK, the red LED comes on and the program stops there: that separates an I2C problem from an LCD problem.
- Then it initializes the LCD, prints both lines, and rewrites the number every 500 ms.

**Check:** build, flash, and run. Line 1 shows **ENCS4110 Lab** and line 2 **Count:** followed by a number that goes up twice a second. If something's wrong, the board tells you which layer:

- **Red LED on:** the I2C layer. The backpack didn't answer at `LCD_ADDR`.
- **No red LED, but the boxes stay on line 1:** the LCD layer. The bytes arrive, but the initialization didn't work.

**Then try** what the ACK means: change `LCD_ADDR` to `0x26` and re-flash. Now nothing answers, the address gets a NACK, and the red LED comes on. Put it back to `0x27`.

::: details The complete program
<<< @/snippets/i2c/main.c#full{c}
:::

::: info How slow is it?
One character is two nibbles, six I2C bytes: about **1.2 ms**. A full 16-character line takes about 20 ms, against microseconds for the parallel LCD of Experiment 8. That's the price of saving five pins, and the reason to update only what changes on the screen.
:::

## Common problems

| Symptom | Cause and fix |
| --- | --- |
| Red LED on | The backpack didn't ACK. Check SDA on **PB3** and SCL on **PB2** (not swapped), `VCC` on VBUS, and that `PCTL` is 3, not 7. |
| Program hangs in `I2C0_Write` | The module isn't running the bus: `RCGCI2C` or `MCR_MFE` is missing, or `AFSEL` isn't set for PB2 and PB3. |
| Backlight on, but boxes stay on line 1 | The initialization didn't reach the LCD. Check `LCD_SendNibble()` puts the nibble in bits 7:4 and pulses `LCD_EN`. |
| Display blank, no boxes, no text | Contrast: turn the blue trimmer on the back of the backpack while the program runs. |
| Random characters | A nibble went missing: the delays after the first `0x03` (5 ms) and after Clear (2 ms) are too short or missing. |
| Backlight goes off when text is printed | `LCD_BL` is missing from the byte in `LCD_SendNibble()`. |

## Task

Build an **RGB mixer** with a menu on the LCD. The knob from Experiment 12 sets each colour's level, pressing the knob picks which colour you're changing, and the RGB LED shows the mix. It brings together the PWM of Experiment 11, the QEI of Experiment 12, and this experiment's LCD.

The display looks like this, with the arrow before the colour you're changing and a bar of its level on line 2 (16 blocks is 100%):

![The RGB mixer display: line 1 reads R050 G020 B099 with an arrow before R; line 2 is a bar of 8 solid blocks for red's 50 percent](/images/exp13/rgb-mixer-lcd.svg)

**Requirements:**
- Drive the three LEDs with PWM through `set_color()` from the [Experiment 11 Task](./exp11-pwm-timers#task).
- Read the knob with QEI1 as in the [Experiment 12 Task](./exp12-qei#task): 1% per click when slow, 5% per click when faster than 120 rpm, each level kept within 1–99%.
- Wire `SW` to **PC4**. Each press moves the arrow to the next colour: red, green, blue, then back to red.
- Show the three levels on line 1, always as three digits, with `>` before the selected colour and a space before the others. Show the selected colour's level as a bar on line 2: `level * 16 / 100` solid blocks (character `0xFF`), the rest spaces.
- Update the LCD **only when something changed**, and only the part that changed: a level change rewrites that colour's three digits and the bar; a press moves the arrow and redraws the bar.
- Don't call `SysTick_Config()`: the LCD's delays use SysTick. The QEI's own interrupt is enough for the speed.

**Wiring:** the LCD as in this Activity, and the HW-040 as in the [Experiment 12 Task](./exp12-qei#task) (CLK on PC5, DT on PC6, SW on PC4, `+` on 3.3V).

::: details Hint: structure
Keep the state in a few variables, and let the main loop compare against what's on the screen:

```c
volatile int32_t level[3] = {50, 20, 99};   // R, G, B in percent: signed, like pwm_set()
uint32_t sel = 0;                           // 0 = R, 1 = G, 2 = B
```

In each pass of the loop: read the clicks and apply them to `level[sel]`; check the button; if anything changed, call `set_color(level[0], level[1], level[2])` and redraw only what changed. Drawing a colour's field means `LCD_SetCursor(0, 5 * i + 2)` and three digits; the arrow is at column `5 * i`.
:::

::: details Hint: three digits and the bar
For a level `n` from 0 to 99, the three digits are `'0' + n / 100`, `'0' + (n / 10) % 10`, and `'0' + n % 10`, each sent with `LCD_Data()`. For the bar, `LCD_SetCursor(1, 0)` and send 16 characters: `0xFF` while the column is below `level * 16 / 100`, then `' '`. Always sending all 16 means a shorter bar overwrites the end of a longer one.
:::

::: details Hint: a faster LCD
A full bar costs 16 characters, 96 I2C transfers. If the knob feels laggy, make `LCD_SendNibble()` send its three bytes in **one** transfer: `MCS_START | MCS_RUN` for the first byte (with the address in `MSA`), `MCS_RUN` for the second, and `MCS_STOP | MCS_RUN` for the third, waiting for `BUSY` after each. That sends the address once instead of three times.
:::
