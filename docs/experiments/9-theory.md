---
outline: deep
lastUpdated: true
prev:
  text: "8. Analog-to-Digital Converter (ADC) - Procedure"
  link: /experiments/8-procedure
next:
  text: "9. UART Serial Communication - Procedure"
  link: /experiments/9-procedure
---

# Experiment 9: Universal Asynchronous Receiver-Transmitter (UART)

::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 <a href="../manual/encs4110_9.pdf" target="_blank">Download Experiment 9 PDF</a>
:::
## Learning Objectives

After completing this experiment, you will be able to:

- Understand the principles of asynchronous serial communication including start bits, stop bits, and data frames.
- Configure UART baud rate using integer and fractional divisors for precise timing.
- Configure GPIO pins for UART alternate function using AFSEL, PCTL, and DEN registers.
- Implement basic UART transmission and reception functions in C.
- Send and receive strings via serial communication using polling methods.
- Interface with a PC terminal emulator for debugging and data visualization.
- Calculate and verify baud rate divisors for different system clock frequencies.
- Understand UART FIFO buffering and status flag operations.

## Experiment Overview

This experiment introduces serial communication using the TM4C123's UART modules. You will configure UART0 for 115200 baud operation on pins PA0 (RX) and PA1 (TX), implement character and string transmission functions, and create an echo server that receives data from a PC terminal and echoes it back. By the end of this lab, you will understand how asynchronous serial communication enables data exchange between embedded systems and external devices, forming the foundation for debugging, sensor interfacing, and wireless communication modules.



## Theoretical Background

### Introduction to Serial Communication

Serial communication is a method of transmitting data one bit at a time over a single communication line, as opposed to parallel communication which sends multiple bits simultaneously. It is widely used in embedded systems due to its simplicity, low pin count, and ability to communicate over long distances.

#### Asynchronous vs Synchronous Communication

Serial communication can be classified into two categories:

- **Asynchronous (UART)**: No shared clock signal; timing is established by agreed-upon baud rate. Uses start and stop bits for synchronization.
- **Synchronous (SPI, I²C)**: Dedicated clock line synchronizes transmitter and receiver. More efficient but requires additional pin.

UART (Universal Asynchronous Receiver-Transmitter) is an asynchronous protocol that requires only two wires: TX (transmit) and RX (receive). It is commonly used for debugging, GPS modules, Bluetooth communication, and PC interfacing.

### UART Frame Structure

A UART transmission consists of a **data frame** that includes:

1. **Start Bit**: Logic low (0) that signals the beginning of transmission
2. **Data Bits**: 5 to 9 bits of actual data (typically 8 bits)
3. **Parity Bit**: Optional error-checking bit (even, odd, or none)
4. **Stop Bits**: Logic high (1) that signals the end of transmission (1, 1.5, or 2 bits)

The most common configuration is **8N1**: 8 data bits, No parity, 1 stop bit.

### Baud Rate Generation

The **baud rate** defines the number of bits transmitted per second, measured in bits per second (bps). Common baud rates include 9600, 19200, 38400, 57600, and 115200 bps. Both transmitter and receiver must use the same baud rate for successful communication.

#### Baud Rate Divisor Calculation

The TM4C123 UART uses a **22-bit baud rate divisor (BRD)** consisting of a 16-bit integer part (BRDI) and a 6-bit fractional part (BRDF). This fractional divisor allows the UART to generate all standard baud rates with high precision.

The baud rate divisor is calculated from the system clock frequency as:

```
BRD = BRDI + BRDF = UARTSysClk / (ClkDiv × BaudRate)
```

where:

- **UARTSysClk**: System clock connected to UART (typically 50 MHz)
- **ClkDiv**: Clock divider (16 if HSE = 0 in UARTCTL, or 8 if HSE = 1)
- **BaudRate**: Desired baud rate (e.g., 115200 bps)

The integer part (BRDI) is loaded into the `UARTIBRD` register, and the fractional part is calculated and loaded into the `UARTFBRD` register using:

```
UARTFBRD[DIVFRAC] = integer(BRDF × 64 + 0.5)
```

The multiplication by 64 and addition of 0.5 accounts for the 6-bit fractional representation and rounding errors.

#### Calculation Example

**Example**: For 115200 baud with 50 MHz system clock and ClkDiv = 16 (default, HSE = 0):

```
BRD = 50,000,000 / (16 × 115,200) = 50,000,000 / 1,843,200 = 27.1267361
BRDI = 27
BRDF = 0.1267361
UARTFBRD[DIVFRAC] = integer(0.1267361 × 64 + 0.5)
                 = integer(8.111 + 0.5) = integer(8.611) = 8
```

Therefore: `UARTIBRD = 27` and `UARTFBRD = 8`

The actual baud rate achieved can be verified:

```
Actual Baud Rate = 50,000,000 / (16 × (27 + 8/64)) = 50,000,000 / (16 × 27.125) = 115,207.4 bps
```

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

**FIFO Status Monitoring:**

FIFO status can be monitored using two registers:

1. **UARTFR (Flag Register)**: Contains empty and full flags
   - **TXFE** (bit 7): Transmit FIFO Empty
   - **TXFF** (bit 5): Transmit FIFO Full
   - **RXFE** (bit 4): Receive FIFO Empty
   - **RXFF** (bit 6): Receive FIFO Full

#### UART Pin Mapping

Each UART module is connected to specific GPIO pins. The table below shows the default pin assignments:

| UART Module | TX Pin | RX Pin |
|-------------|--------|--------|
| UART0 | PA1 | PA0 |
| UART1 | PB1 | PB0 |
| UART2 | PD7 | PD6 |
| UART3 | PC7 | PC6 |
| UART4 | PC5 | PC4 |
| UART5 | PE5 | PE4 |
| UART6 | PD5 | PD4 |
| UART7 | PE1 | PE0 |

**Note**: UART0 on PA0/PA1 is commonly used because it connects to the LaunchPad's on-board USB-to-Serial converter, allowing direct PC communication via USB cable.

### UART Registers

The TM4C123 UART modules are configured through memory-mapped registers.

#### RCGCUART — UART Run Mode Clock Gating Control

Enables clock for UART modules. Each bit corresponds to a UART module (1 = enable, 0 = disable).

#### UARTCTL — UART Control

Configures UART operation including enable, transmit enable, and receive enable.

Where:

- **UARTEN** (bit 0): UART Enable (1 = enable, 0 = disable)
- **TXE** (bit 8): Transmit Enable (1 = enable transmitter)
- **RXE** (bit 9): Receive Enable (1 = enable receiver)
- **HSE** (bit 5): High-Speed Enable (1 = baud rate clock = SysClk/8, 0 = SysClk/16)

#### UARTIBRD — UART Integer Baud-Rate Divisor

Contains the integer part of the baud rate divisor.

Where:

- **DIVINT**: Integer baud rate divisor (0-65535)

#### UARTFBRD — UART Fractional Baud-Rate Divisor

Contains the fractional part of the baud rate divisor.

Where:

- **DIVFRAC**: Fractional baud rate divisor (0-63)

#### UARTLCRH — UART Line Control

Configures data format including word length, FIFO enable, parity, and stop bits.

Where:

- **WLEN** (bits 6-5): Word Length (0x3 = 8 bits, 0x2 = 7 bits, 0x1 = 6 bits, 0x0 = 5 bits)
- **FEN** (bit 4): FIFO Enable (1 = enable 16-byte FIFOs, 0 = disable)
- **STP2** (bit 3): Two Stop Bits (1 = two stop bits, 0 = one stop bit)
- **EPS** (bit 2): Even Parity Select (1 = even parity, 0 = odd parity)
- **PEN** (bit 1): Parity Enable (1 = enable parity, 0 = no parity)
- **BRK** (bit 0): Send Break (1 = send break condition)

**Common Configuration (8N1 with FIFO):**

- WLEN = 0x3 (8 bits)
- FEN = 1 (FIFO enabled)
- STP2 = 0 (one stop bit)
- PEN = 0 (no parity)

Result: `UARTLCRH = 0x60` (0b01100000)

#### UARTCC — UART Clock Configuration

Selects the clock source for UART baud rate generation.

Where:

- **CS** (bits 3-0): Clock Source
  - 0x0 = System clock (default, recommended)
  - 0x5 = PIOSC (16 MHz internal oscillator)

#### UARTFR — UART Flag Register

Contains status flags for transmit/receive operations.

Where:

- **TXFE** (bit 7): Transmit FIFO Empty (1 = empty, 0 = not empty)
- **RXFF** (bit 6): Receive FIFO Full (1 = full, 0 = not full)
- **TXFF** (bit 5): Transmit FIFO Full (1 = full, 0 = not full)
- **RXFE** (bit 4): Receive FIFO Empty (1 = empty, 0 = not empty)
- **BUSY** (bit 3): UART Busy (1 = transmitting, 0 = idle)

**Usage:**

- Before transmitting: Wait while `TXFF = 1` (transmit FIFO full)
- Before receiving: Wait while `RXFE = 1` (receive FIFO empty)

#### UARTDR — UART Data Register

Contains the data to transmit or received data. Reading removes data from RX FIFO; writing adds to TX FIFO.

Where: Data is contained in the lower 8 bits. The upper 4 bits indicate error flags:

- **FE** (bit 3): Framing Error
- **PE** (bit 2): Parity Error
- **BE** (bit 1): Break Error
- **OE** (bit 0): Overrun Error
- **DATA** (bits 7-0): Received or Transmit Data