---
title: "Universal Asynchronous Receiver-Transmitter (UART)"
description: "This experiment introduces serial communication using the TM4C123’s UART modules."
outline: deep
lastUpdated: true
manual: "/manual/encs4110_manual.pdf#nameddest=chapter.10"
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=893"
current: "/experiments/exp10-uart"
short: "UART"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Universal Asynchronous Receiver-Transmitter (UART)

This experiment introduces serial communication using the TM4C123’s UART modules. You will configure UART0 for 115200 baud operation on pins PA0 (RX) and PA1 (TX), implement character and string transmission functions, and create an echo server that receives data from a PC terminal and echoes it back. By the end of this lab, you will understand how asynchronous serial communication enables data exchange between embedded systems and external devices, forming the foundation for debugging, sensor interfacing, and wireless communication modules.

::: warning Preview: new format
This is the reworked version of [Experiment 10](/experiments/exp10-uart), shared for TAs to test. It has the same background, then one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp10-uart)** until this one replaces it.
:::


## Objectives

- Understand the principles of asynchronous serial communication including start bits, stop bits, and data frames.
- Configure UART baud rate using integer and fractional divisors for precise timing.
- Configure GPIO pins for UART alternate function using AFSEL, PCTL, and DEN registers.
- Implement basic UART transmission and reception functions in C.
- Send and receive strings via serial communication using polling methods.
- Interface with a PC terminal emulator for debugging and data visualization.
- Calculate and verify baud rate divisors for different system clock frequencies.
- Understand UART FIFO buffering and status flag operations.

## Background

### Introduction to Serial Communication

Serial communication is a method of transmitting data one bit at a time over a single communication line, as opposed to parallel communication which sends multiple bits simultaneously. It is widely used in embedded systems due to its simplicity, low pin count, and ability to communicate over long distances.

#### Asynchronous vs Synchronous Communication

Serial communication can be classified into two categories:

- **Asynchronous (UART)**: No shared clock signal; timing is established by agreed-upon baud rate. Uses start and stop bits for synchronization.
- **Synchronous (SPI, I²C)**: Dedicated clock line synchronizes transmitter and receiver. More efficient but requires additional pin.

UART (Universal Asynchronous Receiver-Transmitter) is an asynchronous protocol that requires only two wires: TX (transmit) and RX (receive). It is commonly used for debugging, GPS modules, Bluetooth communication, and PC interfacing.

### UART Frame Structure

A UART transmission consists of a **data frame** that includes:

1.  **Start Bit**: Logic low (0) that signals the beginning of transmission
2.  **Data Bits**: 5 to 9 bits of actual data (typically 8 bits)
3.  **Parity Bit**: Optional error-checking bit (even, odd, or none)
4.  **Stop Bits**: Logic high (1) that signals the end of transmission (1, 1.5, or 2 bits in the general async-serial standard; the TM4C123’s `UARTLCRH` only supports 1 or 2 via the `STP2` bit — see below)

The most common configuration is **8N1**: 8 data bits, No parity, 1 stop bit.

![UART 8N1 frame: a low start bit, data bits D0 to D7 least significant first, then a high stop bit](/images/manual/uart-frame.svg)

*UART frame structure (8N1 format)*

### Baud Rate Generation

The **baud rate** defines the number of bits transmitted per second, measured in bits per second (bps). Common baud rates include 9600, 19200, 38400, 57600, and 115200 bps. Both transmitter and receiver must use the same baud rate for successful communication.

#### Baud Rate Divisor Calculation

The TM4C123 UART uses a **22-bit baud rate divisor (BRD)** consisting of a 16-bit integer part (BRDI) and a 6-bit fractional part (BRDF). This fractional divisor allows the UART to generate all standard baud rates with high precision.

The baud rate divisor is calculated from the system clock frequency as:

$$
\text{BRD} = \text{BRDI} + \text{BRDF} = \frac{\text{UARTSysClk}}{\text{ClkDiv} \times \text{BaudRate}}
$$

where:

- **UARTSysClk**: System clock connected to UART (typically 50 MHz)
- **ClkDiv**: Clock divider (16 if HSE = 0 in UARTCTL, or 8 if HSE = 1)
- **BaudRate**: Desired baud rate (e.g., 115200 bps)

The integer part (BRDI) is loaded into the `UARTIBRD` register, and the fractional part is calculated and loaded into the `UARTFBRD` register using:

$$
\text{UARTFBRD[DIVFRAC]} = \text{integer}(\text{BRDF} \times 64 + 0.5)
$$

The multiplication by 64 and addition of 0.5 accounts for the 6-bit fractional representation and rounding errors.

#### Calculation Example

**Example**: For 115200 baud with 50 MHz system clock and ClkDiv = 16 (default, HSE = 0):

$$
\begin{aligned}
\text{BRD} &= \frac{50{,}000{,}000}{16 \times 115{,}200} = \frac{50{,}000{,}000}{1{,}843{,}200} = 27.1267361 \\
\text{BRDI} &= 27 \\
\text{BRDF} &= 0.1267361 \\
\text{UARTFBRD[DIVFRAC]} &= \text{integer}(0.1267361 \times 64 + 0.5) \\
&= \text{integer}(8.111 + 0.5) = \text{integer}(8.611) = 8
\end{aligned}
$$

Therefore: `UARTIBRD = 27` and `UARTFBRD = 8`

The actual baud rate achieved can be verified:
$$
\text{Actual Baud Rate} = \frac{50{,}000{,}000}{16 \times (27 + 8/64)} = \frac{50{,}000{,}000}{16 \times 27.125} = 115{,}207.4 \text{ bps}
$$

This gives an error of only 0.0064%, well within acceptable tolerance for reliable communication.

### TM4C123 UART Modules

The TM4C123GH6PM provides **eight UART modules** (UART0-UART7) with the following features:

- **Programmable Baud Rate**: Generated from system clock with 16-bit divisor
- **Data Format**: 5 to 9 data bits, optional parity (even, odd, stick), 1 or 2 stop bits
- **FIFO Buffers**: 16-byte transmit and receive FIFOs for reduced interrupt overhead
- **Full-Duplex Operation**: Simultaneous transmission and reception
- **Error Detection**: Framing, parity, overrun, and break detection
- **Interrupt Generation**: Configurable interrupts for TX, RX, and error conditions
- **DMA Support**: μDMA channels for high-speed data transfer
- **IrDA and ISO 7816**: Optional infrared and smart card support
- **9-Bit Mode**: For RS-485 multi-drop networks

#### FIFO Operation

Each UART module contains two 16x8 FIFOs: one for transmit and one for receive. Both FIFOs are accessed through the `UARTDR` register. Read operations return a 12-bit value consisting of 8 data bits and 4 error flags, while write operations place 8-bit data into the transmit FIFO.

**FIFO Status Monitoring**:

FIFO status can be monitored using two registers:

1.  **UARTFR (Flag Register)**: Contains empty and full flags

    - **TXFE** (bit 7): Transmit FIFO Empty

    - **TXFF** (bit 5): Transmit FIFO Full

    - **RXFE** (bit 4): Receive FIFO Empty

    - **RXFF** (bit 6): Receive FIFO Full

#### UART Pin Mapping

Each UART module is connected to specific GPIO pins. The table below shows the default pin assignments:

| **UART Module** | **TX Pin** | **RX Pin** |
|:---------------:|:----------:|:----------:|
|      UART0      |    PA1     |    PA0     |
|      UART1      |    PB1     |    PB0     |
|      UART2      |    PD7     |    PD6     |
|      UART3      |    PC7     |    PC6     |
|      UART4      |    PC5     |    PC4     |
|      UART5      |    PE5     |    PE4     |
|      UART6      |    PD5     |    PD4     |
|      UART7      |    PE1     |    PE0     |

*UART Pin Assignments*

**Note**: UART0 on PA0/PA1 is commonly used because it connects to the LaunchPad’s on-board USB-to-Serial converter, allowing direct PC communication via USB cable.

### UART Registers

The TM4C123 UART modules are configured through memory-mapped registers. Key registers include:

#### RCGCUART — UART Run Mode Clock Gating Control

Enables clock for UART modules. Each bit corresponds to a UART module (1 = enable, 0 = disable).

**RCGCUART Register — UART Run Mode Clock Gating Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | R7 |
| 6 | R6 |
| 5 | R5 |
| 4 | R4 |
| 3 | R3 |
| 2 | R2 |
| 1 | R1 |
| 0 | R0 |

#### UARTCTL — UART Control

Configures UART operation including enable, transmit enable, and receive enable.

**UARTCTL Register — UART Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15 | CTSEN |
| 14 | RTSEN |
| 13–12 | reserved |
| 11 | RTS |
| 10 | reserved |
| 9 | RXE |
| 8 | TXE |
| 7 | LBE |
| 6 | reserved |
| 5 | HSE |
| 4 | EOT |
| 3 | SMART |
| 2 | SIRLP |
| 1 | SIREN |
| 0 | UARTEN |

*Where:*

- **UARTEN** (bit 0): UART Enable (1 = enable, 0 = disable)
- **TXE** (bit 8): Transmit Enable (1 = enable transmitter)
- **RXE** (bit 9): Receive Enable (1 = enable receiver)
- **HSE** (bit 5): High-Speed Enable (1 = baud rate clock = SysClk/8, 0 = SysClk/16)

#### UARTIBRD — UART Integer Baud-Rate Divisor

Contains the integer part of the baud rate divisor.

**UARTIBRD Register — UART Integer Baud-Rate Divisor**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–0 | DIVINT |

*Where:*

- **DIVINT**: Integer baud rate divisor (0-65535)

#### UARTFBRD — UART Fractional Baud-Rate Divisor

Contains the fractional part of the baud rate divisor.

**UARTFBRD Register — UART Fractional Baud-Rate Divisor**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–6 | reserved |
| 5–0 | DIVFRAC |

*Where:*

- **DIVFRAC**: Fractional baud rate divisor (0-63)

#### UARTLCRH — UART Line Control

Configures data format including word length, FIFO enable, parity, and stop bits.

**UARTLCRH Register — UART Line Control**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | SPS |
| 6–5 | WLEN |
| 4 | FEN |
| 3 | STP2 |
| 2 | EPS |
| 1 | PEN |
| 0 | BRK |

*Where:*

- **WLEN** (bits 6-5): Word Length (0x3 = 8 bits, 0x2 = 7 bits, 0x1 = 6 bits, 0x0 = 5 bits)
- **FEN** (bit 4): FIFO Enable (1 = enable 16-byte FIFOs, 0 = disable)
- **STP2** (bit 3): Two Stop Bits (1 = two stop bits, 0 = one stop bit)
- **EPS** (bit 2): Even Parity Select (1 = even parity, 0 = odd parity)
- **PEN** (bit 1): Parity Enable (1 = enable parity, 0 = no parity)
- **BRK** (bit 0): Send Break (1 = send break condition)

**Common Configuration (8N1 with FIFO)**:

- WLEN = 0x3 (8 bits)
- FEN = 1 (FIFO enabled)
- STP2 = 0 (one stop bit)
- PEN = 0 (no parity)

Result: `UARTLCRH = 0x70` (0b01110000) — WLEN (bits 6-5) contributes 0x60, FEN (bit 4) adds 0x10.

#### UARTCC — UART Clock Configuration

Selects the clock source for UART baud rate generation.

**UARTCC Register — UART Clock Configuration**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–4 | reserved |
| 3–0 | CS |

*Where:*

- **CS** (bits 3-0): Clock Source

  - 0x0 = System clock (default, recommended)

  - 0x5 = PIOSC (16 MHz internal oscillator)

#### UARTFR — UART Flag Register

Contains status flags for transmit/receive operations.

**UARTFR Register — UART Flag Register**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–8 | reserved |
| 7 | TXFE |
| 6 | RXFF |
| 5 | TXFF |
| 4 | RXFE |
| 3 | BUSY |
| 2–1 | reserved |
| 0 | CTS |

*Where:*

- **TXFE** (bit 7): Transmit FIFO Empty (1 = empty, 0 = not empty)
- **RXFF** (bit 6): Receive FIFO Full (1 = full, 0 = not full)
- **TXFF** (bit 5): Transmit FIFO Full (1 = full, 0 = not full)
- **RXFE** (bit 4): Receive FIFO Empty (1 = empty, 0 = not empty)
- **BUSY** (bit 3): UART Busy (1 = transmitting, 0 = idle)

**Usage**:

- Before transmitting: Wait while `TXFF = 1` (transmit FIFO full)
- Before receiving: Wait while `RXFE = 1` (receive FIFO empty)

#### UARTDR — UART Data Register

Contains the data to transmit or received data. Reading removes data from RX FIFO; writing adds to TX FIFO.

**UARTDR Register — UART Data Register**

| Bits | Field |
| --- | --- |
| 31–16 | reserved |
| 15–12 | reserved |
| 11 | OE |
| 10 | BE |
| 9 | PE |
| 8 | FE |
| 7–0 | DATA |

*Where: Data is contained in the lower 8 bits. The upper 4 bits indicate error flags:*

- **FE** (bit 8): Framing Error
- **PE** (bit 9): Parity Error
- **BE** (bit 10): Break Error
- **OE** (bit 11): Overrun Error
- **DATA** (bits 7-0): Received or Transmit Data

## Activity

Talk to the LaunchPad from your PC. The board prints a greeting and a `> ` prompt; you type a line in a terminal, and the board echoes each character as you type, then answers with what it received. UART0 is wired to the LaunchPad's USB connection, so the same cable that flashes the board carries the serial data: no extra wiring.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **C**, and make sure its blinking-LED test works. All the code goes in that project's `main.c`.
:::

### Step 1: Open a terminal

1. With the LaunchPad plugged in, open **Device Manager → Ports (COM & LPT)** on Windows and note the number of **Stellaris Virtual Serial Port** (for example `COM5`).
2. Open [PuTTY](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html), choose **Serial**, enter that COM port, and set **Speed** to **115200**. The other settings stay at their defaults: 8 data bits, no parity, 1 stop bit, no flow control.

![PuTTY serial configuration: connection type Serial, the LaunchPad's COM port, speed 115200](/putty.png)

Keep PuTTY open while you work: Keil can flash the board with the terminal connected.

### Step 2: Add the definitions

Replace the contents of `main.c` with the include and the definitions:

<<< @/snippets/preview/exp10/main.c#defines{c}

- **`PCTL_PA01_UART`** puts 1 in the `PCTL` fields of PA0 and PA1: the [UART pin mapping](#uart-pin-mapping) for U0RX and U0TX.
- **`LCRH_WLEN_8`** is the 2-bit word-length field at bits 6:5 set to 3: 8 data bits. Parity and the second stop bit stay off, so this is "8N1", what PuTTY uses by default.
- **`BAUD_IBRD`** and **`BAUD_FBRD`** come from the [baud rate divisor](#baud-rate-divisor-calculation): $50{,}000{,}000 / (16 \times 115200) = 27.1267$. The integer part goes in `IBRD`; the fraction times 64, rounded, goes in `FBRD`.

### Step 3: Add the init and send functions

Add these after the definitions:

<<< @/snippets/preview/exp10/main.c#send{c}

- **`UART0_Init()`** first hands PA0 and PA1 to UART0, then configures it while it's off: baud divisors, then `LCRH`. The order matters: writing `LCRH` is what makes the UART take the new divisors.
- **`UART0_WriteChar()`** waits while the transmit FIFO is full (`TXFF`), then writes the character. The FIFO holds 16 characters, so the wait only happens when you send faster than 115200 baud can carry them.
- **`UART0_WriteString()`** sends characters until the string's terminating 0.

### Step 4: Add the receive functions

Add these after the send functions:

<<< @/snippets/preview/exp10/main.c#receive{c}

- **`UART0_ReadChar()`** waits while the receive FIFO is empty (`RXFE`), then reads one character. `DR` has error flags in bits 11:8, so only the low byte is kept.
- **`UART0_ReadLine()`** collects characters until Enter. A terminal doesn't show what you type by itself: the board has to **echo** each character back. Backspace is handled by moving the cursor back, printing a space over the character, and moving back again.
- PuTTY's Enter sends `\r` (carriage return), and Backspace sends `0x7F`. The function accepts `\n` and `\b` too, for other terminals.

### Step 5: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp10/main.c#main{c}

Lines sent to the terminal end in `"\r\n"`: `\r` returns the cursor to the start of the line, and `\n` moves it down one.

**Check:** build and flash, then press the LaunchPad's **RESET** button with PuTTY open. You should see **ENCS4110 UART ready** and a `> ` prompt. Type `hello`: each letter appears as you type it. Press Enter: the board answers **You typed: hello**. Try Backspace in the middle of a word.

**Then try:**

- Remove the `UART0_WriteChar(c);` echo from `UART0_ReadLine()`: you now type blind, though the board still receives everything.
- Change `BAUD_IBRD` to 54 (half the baud rate) without changing PuTTY: the output turns to garbage. Both ends must agree on the baud rate, because there's no clock wire.
- Type more than 31 characters: the extra ones are ignored, because `line` has room for 31 and the terminating 0.

::: details The complete program
<<< @/snippets/preview/exp10/main.c#full{c}
:::

## Common problems

| Symptom | Cause and fix |
| --- | --- |
| Nothing appears in PuTTY | Wrong COM port, or PuTTY isn't set to 115200. Press RESET after opening PuTTY: the greeting was sent before you connected. |
| Garbage characters | Baud rate mismatch: check `IBRD` and `FBRD`, and that `LCRH` is written **after** them. |
| PuTTY can't open the port | Another program has it, such as a second PuTTY window. Close it. |
| Output appears, typing does nothing | The receiver is off: `CTL_RXE` is missing from `CTL`. |
| Lines start further and further right | `"\n"` without `"\r"`: send `"\r\n"` at the end of every line. |
| Program stops in `HardFault_Handler` | A UART0 register was written before its clock was ready: keep the `PRUART` wait. |

## Task

Build a **command shell**: the board reads a command per line, does it, and answers. It ties together the GPIO of [Experiment 5](/preview/exp5-gpio-output), the ADC of [Experiment 9](/preview/exp9-adc), and this experiment's UART.

| Command | What it does | Example answer |
|:---|:---|:---|
| `help` | Lists the commands | `led <red\|green\|blue\|off>, adc, count, help` |
| `led red` (or `green`, `blue`) | Lights that LED, turning the others off | `OK` |
| `led off` | Turns all the LEDs off | `OK` |
| `adc` | Reads the potentiometer on PE3 (AIN0), as in Experiment 9 | `1650 mV` |
| `count` | How many commands have been run since reset | `7` |
| anything else | | `Unknown command: blink. Type help.` |

**Requirements:**
- Read each command with `UART0_ReadLine()` and compare it with the known commands.
- Write `UART0_WriteNumber(uint32_t n)` to print numbers, the same way `LCD_PrintNumber()` does in [Experiment 8](/preview/exp8-lcd).
- Accept commands typed in any mix of upper and lower case: `LED Red` works like `led red`.
- Keep the command handling in one function, `run_command(const char *line)`, so `main` stays a short loop.

::: details Hint: comparing strings
`#include <string.h>` gives you `strcmp(a, b)`, which returns 0 when two strings are equal, and `strncmp(a, b, n)`, which compares only the first `n` characters: `strncmp(line, "led ", 4) == 0` finds every `led` command, and `line + 4` is the colour after it. For case, convert the line to lower case once, before comparing: a character `c` between `'A'` and `'Z'` becomes `c + ('a' - 'A')`.
:::
