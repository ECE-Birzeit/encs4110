---
outline: deep
lastUpdated: true
prev:
  text: "5. GPIO Inputs and Interrupt Handling - Procedure"
  link: /experiments/5-procedure
next:
  text: "6. Hardware Timers and Timing Control - Procedure"
  link: /experiments/6-procedure
---
# Experiment 6: Hardware Timers and Timing Control

::: info Complete Lab Manual
For the complete experiment including learning objectives, theoretical background, and detailed explanations, download the PDF manual:
 <a href="/manual/encs4110_6.pdf" target="_blank">Download Experiment 6 PDF</a>
:::

## Learning Objectives

After completing this experiment, you will be able to:

- Understand the role and limitations of the SysTick timer in the ARM Cortex-M4 core.
- Configure the SysTick timer for periodic interrupts and precise timing.
- Understand the architecture and capabilities of General-Purpose Timer Modules (GPTM).
- Configure GPTM in 16-bit and 32-bit modes for periodic and one-shot operation.
- Calculate timer periods using prescalers and reload values.
- Implement timer-based debouncing for mechanical switches.
- Use multiple timers to control independent timing tasks.

## Experiment Overview

This experiment introduces precise timing control using the TM4C123's SysTick timer and General-Purpose Timer Modules (GPTM). You will configure SysTick for periodic interrupts and millisecond timing, explore GPTM's 16-bit and 32-bit modes with prescalers, and implement multiple independent timing tasks. By the end of this lab, you will understand how to select and configure timers for different timing requirements and implement timer-based interrupt-driven applications.



## Theoretical Background

### Introduction to Timers

Hardware timers are specialized peripherals that count clock cycles and generate events at precise intervals. Unlike software-based delays (which block the CPU and waste power), hardware timers run independently in the background and generate interrupts when they reach a predetermined count.

#### Why Use Hardware Timers?

- **Precise timing**: Timers operate at the system clock frequency (50 MHz), providing microsecond-level accuracy
- **Non-blocking**: The CPU can perform other tasks while the timer runs independently
- **Power efficient**: Timers can wake the MCU from sleep mode
- **Multiple independent channels**: Several timers can run simultaneously with different periods

#### When to Use Each Timer

| Feature | SysTick | GPTM |
|---------|---------|------|
| **Bit width** | 24-bit | 16-bit or 32-bit |
| **Max period @ 50MHz** | ~335 ms | 32-bit: ~86 seconds |
| **Use case** | OS ticks, simple delays | Complex timing, PWM, capture |
| **Availability** | Built into ARM Cortex-M core | 6-12 on TM4C123 |

For short delays (< 300 ms), SysTick is simpler. For longer delays or advanced features, use GPTM.


### Timer Fundamentals

Understanding how timers work internally helps you configure them correctly and debug timing issues.

#### How Timers Operate

Both SysTick and GPTM timers operate as **down-counters** in periodic mode:

1. **Load**: A reload value is written to the timer's load register (`LOAD` for SysTick, `TAILR` for GPTM)
2. **Count**: The timer counts down from the reload value to zero, decrementing once per clock cycle
3. **Timeout**: When the counter reaches zero:
   - The timeout flag is set
   - An interrupt is generated (if enabled)
   - The counter automatically reloads from the load register (periodic mode) or stops (one-shot mode)
4. **Clear**: The interrupt handler must clear the timeout flag


#### Timer Modes

Timers can operate in different modes depending on the application:

- **Periodic Mode**: The timer automatically reloads and continues counting. Ideal for fixed-rate tasks like LED blinking, sensor sampling, or OS ticks.
- **One-Shot Mode**: The timer counts down once and stops. Useful for timeouts, single delays, or one-time events.
- **Capture Mode**: (GPTM only) Captures the counter value when an external event occurs on a GPIO pin. Used for measuring pulse widths or frequencies.
- **PWM Mode**: (GPTM only) Generates pulse-width modulated output signals for motor control, dimming LEDs, etc.


#### Prescaler (16-bit GPTM Mode Only)

In 16-bit mode, GPTM provides an 8-bit prescaler (`TAPR`) that extends the timer range by dividing the input clock:

```
T = (TAILR + 1) × (TAPR + 1) / f_SysClk
```

The prescaler is ignored in 32-bit mode. For example, with `TAPR = 255` (divisor = 256), the effective clock frequency is reduced to 50 MHz / 256 ≈ 195.3 kHz.


#### Interrupt Handling

Proper interrupt handling is critical for timer-based applications:

1. **Enable the interrupt**: Set the appropriate bit in the timer's interrupt mask register (`IMR`)
2. **Enable in NVIC**: Set the corresponding bit in the NVIC's `ISER` register
3. **Implement the ISR**: Write an interrupt service routine with the correct name (e.g., `SysTick_Handler()`, `TIMER1A_Handler()`)
4. **Clear the flag**: Write 1 to the appropriate bit in the interrupt clear register (`ICR`) to acknowledge the interrupt

**Best Practice**: Clear the interrupt flag early in the ISR to prevent missing subsequent interrupts:

```c
void TIMER1A_Handler(void) {
    TIMER1->ICR = (1 << 0);  // Clear TATOCINT flag immediately
    // Perform minimal work: toggle GPIO, update counters, etc.
}
```


### SysTick Timer

The SysTick timer is a 24-bit down-counter that is part of the ARM Cortex-M4 core. It is present in all Cortex-M processors and provides a standard, simple timing mechanism for operating systems and applications.

#### SysTick Features

The SysTick timer provides:

- **24-bit down-counter**: Counts from a reload value down to zero.
- **Automatic reload**: When the counter reaches zero, it automatically reloads from the `LOAD` register.
- **Optional interrupt**: Can generate an interrupt when the counter reaches zero.
- **Clock source selection**: Can use the processor clock or an external reference clock.
- **CMSIS support**: The CMSIS library provides a simple `SysTick_Config()` function for easy setup.

The SysTick timer is ideal for creating system ticks (e.g., 1 ms intervals for RTOS scheduling), implementing delays, and measuring time intervals. However, its 24-bit width limits the maximum period at high clock frequencies.

#### SysTick Registers

The SysTick timer is controlled through three main registers:

##### SysTick Control and Status Register (STCTRL)

**Register:** `SysTick->CTRL` — SysTick Control and Status (`0xE000E010`)

The `CTRL` register controls the SysTick timer operation and provides status information.

**Key Bits:**

- **Bit 0 (ENABLE)**: Enable/disable the SysTick timer
  - 0 = Timer disabled
  - 1 = Timer enabled
- **Bit 1 (INTEN)**: Enable SysTick interrupt
  - 0 = No interrupt when counter reaches zero
  - 1 = Generate interrupt when counter reaches zero
- **Bit 2 (CLK_SRC)**: Clock source selection
  - 0 = External reference clock
  - 1 = Processor clock (typical)
- **Bit 16 (COUNT)**: Counter reached zero since last read (read-only)
  - 0 = Has not counted to zero
  - 1 = Has counted to zero (cleared on read)

##### SysTick Reload Value Register (STLOAD)

**Register:** `SysTick->LOAD` — SysTick Reload Value (`0xE000E014`)

The `LOAD` register holds the value that is loaded into the counter when it reaches zero or when the timer is enabled.

**RELOAD Field (Bits 23:0):** The value to load into the counter. Valid range: `0x000001` to `0xFFFFFF`. Writing zero disables the counter.

##### SysTick Current Value Register (STCURRENT)

**Register:** `SysTick->VAL` — SysTick Current Value (`0xE000E018`)

The `VAL` register contains the current value of the SysTick counter.

**CURRENT Field (Bits 23:0):** Current counter value. Writing any value clears the counter to zero and clears the `COUNTFLAG` in `CTRL`.

#### SysTick Timing Calculation

The SysTick timer counts down from the `LOAD` value to zero. The period is determined by:

```
T = (LOAD + 1) / f_clock
```

where f_clock is the processor clock frequency (typically 50 MHz on the TM4C123).

**Maximum Period:** With a 24-bit counter at 50 MHz:
```
T_max = 2^24 / 50,000,000 = 16,777,216 / 50,000,000 ≈ 0.335 seconds
```

For longer periods, accumulate ticks in software or use the GPTM.

#### SysTick Configuration with CMSIS

The CMSIS library provides a convenient function to configure the SysTick timer:

```c
// Configure SysTick for 1 ms interrupts at SystemCoreClock
SysTick_Config(SystemCoreClock / 1000);  // 50,000 ticks = 1 ms period
```

This function:

- Sets the `LOAD` register to the specified value minus 1
- Clears the `VAL` register
- Enables the SysTick interrupt in the NVIC
- Enables the SysTick timer with the processor clock source

The SysTick interrupt handler is named `SysTick_Handler()`:

```c
void SysTick_Handler(void) {
    // Called every 1 ms
    // Increment global counter, toggle LED, etc.
}
```


### General-Purpose Timer Module (GPTM)

The TM4C123 includes twelve General-Purpose Timer Modules (GPTM): six 16/32-bit timers (TIMER0-TIMER5) and six 32/64-bit wide timers (WTIMER0-WTIMER5). Each module can operate as two independent 16-bit timers or one 32-bit timer.

#### GPTM Features

GPTM modules provide:

- **16-bit or 32-bit operation**: Configurable timer width
- **Two independent timers**: Timer A and Timer B in each module
- **Multiple operating modes**:
  - One-Shot mode: Timer runs once and stops
  - Periodic mode: Timer reloads automatically
  - Real-Time Clock (RTC) mode: Precise time-keeping
  - PWM mode: Pulse Width Modulation output
  - Input Capture mode: Measure input signal timing
- **Prescaler**: Extends timer range in 16-bit mode (8-bit prescaler for standard timers)
- **Interrupt generation**: Timeout, match, and capture interrupts
- **Trigger and synchronization**: Can trigger ADC conversions or synchronize with other timers

#### GPTM Architecture

Each GPTM module contains:

- **Timer A**: Independent timer with its own registers
- **Timer B**: Independent timer with its own registers
- **Configuration Register (CFG)**: Selects 16-bit or 32-bit mode
- **Mode Registers (TAMR, TBMR)**: Configure operating mode for each timer
- **Load Registers (TAILR, TBILR)**: Set the reload/start value
- **Prescaler Registers (TAPR, TBPR)**: Extend timer range (16-bit mode only)
- **Control Register (CTL)**: Enable/disable timers and configure behavior
- **Interrupt Registers (IMR, RIS, MIS, ICR)**: Manage interrupts

In 32-bit mode, Timer A operates as a full 32-bit timer, and Timer B is not available. In 16-bit mode, both Timer A and Timer B operate independently as 16-bit timers.


### GPTM Configuration Registers

#### GPTMCFG — Timer Configuration Register

**Register:** `TIMERx->CFG` — GPTM Configuration (`Base + 0x000`)

The `CFG` register selects the timer width (16-bit or 32-bit mode).

**GPTMCFG Field (Bits 2:0):**

- `0x0`: 32-bit timer configuration
- `0x1`: 32-bit real-time clock (RTC) counter configuration
- `0x2-0x3`: Reserved
- `0x4`: 16-bit timer configuration (controlled by TAMR/TBMR bits 1:0)
- `0x5-0x7`: Reserved

#### GPTMTAMR — Timer A Mode Register

**Register:** `TIMERx->TAMR` — GPTM Timer A Mode (`Base + 0x004`)

The `TAMR` register configures the operating mode for Timer A.

**Key Fields:**

- **Bits 1:0 (TAMR)**: Timer A Mode
  - `0x1`: One-Shot mode
  - `0x2`: Periodic mode
  - `0x3`: Capture mode
- **Bit 4 (TACDIR)**: Timer A Count Direction (0 = down, 1 = up)

#### GPTMTAILR — Timer A Interval Load Register

**Register:** `TIMERx->TAILR` — GPTM Timer A Interval Load (`Base + 0x028`)

The `TAILR` register sets the start/reload value for Timer A.

**TAILR Field:**

- **Count-down mode**: Specifies the starting count value loaded into the timer
- **Count-up mode**: Sets the upper bound for the timeout event
- **32-bit mode**: Full 32-bit register
- **16-bit mode**: Only bits [15:0] are used

#### GPTMTAPR — Timer A Prescaler Register

**Register:** `TIMERx->TAPR` — GPTM Timer A Prescaler (`Base + 0x038`)

The `TAPR` register extends the timer range in 16-bit mode by providing an 8-bit prescaler.

**TAPSR Field (Bits 7:0):**

- Prescaler value: `0x00` to `0xFF` (0 to 255)
- Only used in 16-bit mode; ignored in 32-bit mode
- Effective divisor: `TAPSR + 1`

#### GPTMCTL — Timer Control Register

**Register:** `TIMERx->CTL` — GPTM Control (`Base + 0x00C`)

The `CTL` register enables/disables timers and configures their behavior.

**Key Control Functions:**

- **Timer Enable Control**: TAEN (bit 0) and TBEN (bit 8) independently enable/disable Timer A and Timer B
- **Output Trigger Control**: TAOTE (bit 5) and TBOTE (bit 13) enable timers to trigger external peripherals

#### GPTMIMR — Interrupt Mask Register

**Register:** `TIMERx->IMR` — GPTM Interrupt Mask (`Base + 0x018`)

The `IMR` register enables or disables (masks/unmasks) timer interrupts.

**Key Bits:**

- **Bit 0 (TATOIM)**: Timer A Timeout Interrupt Mask (0 = masked, 1 = enabled)
- **Bit 8 (TBTOIM)**: Timer B Timeout Interrupt Mask

#### GPTMICR — Interrupt Clear Register

**Register:** `TIMERx->ICR` — GPTM Interrupt Clear (`Base + 0x024`)

Writing '1' to a bit in this register clears the corresponding interrupt flag.


### GPTM Timing Calculations

The timer period depends on the operating mode and configuration:

#### 32-bit Mode (No Prescaler)

In 32-bit mode, the period is:

```
T = (TAILR + 1) / f_clock
```

At 50 MHz, the maximum period is:
```
T_max = 2^32 / 50,000,000 = 4,294,967,296 / 50,000,000 ≈ 85.9 seconds
```

#### 16-bit Mode (With Prescaler)

In 16-bit mode, the prescaler extends the timer range:

```
T = (TAILR + 1) × (TAPR + 1) / f_clock
```

At 50 MHz, with maximum values (`TAILR = 0xFFFF`, `TAPR = 0xFF`):
```
T_max = 65,536 × 256 / 50,000,000 = 16,777,216 / 50,000,000 ≈ 0.335 seconds
```

### GPTM Interrupt Configuration

Each GPTM module has a unique interrupt number for Timer A and Timer B. The CMSIS-compliant header `tm4c123gh6pm.h` defines these as `IRQn_Type` enumerations (e.g., `TIMER1A_IRQn`).

#### Enabling an interrupt (CMSIS)

Use `NVIC_EnableIRQ(IRQn_Type irqn)`:

```c
#include "TM4C123.h"

// Enable TIMER1A interrupt:
NVIC_EnableIRQ(TIMER1A_IRQn);

// Enable multiple GPTM interrupts:
NVIC_EnableIRQ(TIMER0A_IRQn);
NVIC_EnableIRQ(TIMER1A_IRQn);
NVIC_EnableIRQ(TIMER2A_IRQn);
```

#### Setting interrupt priority

Use `NVIC_SetPriority(IRQn_Type irqn, uint32_t priority)` before enabling the interrupt. On Cortex-M4 (TM4C123), priorities are typically 0-7 (0 = highest urgency).

```c
// Assign priorities (lower number = higher priority)
NVIC_SetPriority(TIMER0A_IRQn, 3);
NVIC_SetPriority(TIMER1A_IRQn, 4);
NVIC_SetPriority(TIMER2A_IRQn, 5);

// Then enable them
NVIC_EnableIRQ(TIMER0A_IRQn);
NVIC_EnableIRQ(TIMER1A_IRQn);
NVIC_EnableIRQ(TIMER2A_IRQn);
```


### Configuration Workflow

#### SysTick Configuration Steps

To configure the SysTick timer:

1. Calculate the reload value: `LOAD = (Period × SystemCoreClock) - 1`
2. Load the value into `SysTick->LOAD`
3. Clear the current value: `SysTick->VAL = 0`
4. Configure `SysTick->CTRL`:
   - Set bit 0 (ENABLE) to enable the timer
   - Set bit 1 (TICKINT) to enable interrupts
   - Set bit 2 (CLKSOURCE) to use processor clock
5. Implement `SysTick_Handler()` to handle interrupts

Or use the CMSIS function:
```c
SysTick_Config(SystemCoreClock / 1000);  // 1 ms period
```

#### GPTM Configuration Steps (Periodic Mode)

To configure a GPTM timer in periodic mode:

1. Enable the timer clock in `SYSCTL->RCGCTIMER`
2. Wait for clock stabilization (3 NOP instructions or check ready bit)
3. Disable the timer: `TIMERx->CTL = 0`
4. Select timer width: `TIMERx->CFG` (`0x00` for 32-bit, `0x04` for 16-bit)
5. Configure mode: `TIMERx->TAMR = 0x02` (periodic mode, count down)
6. Set prescaler (16-bit mode only): `TIMERx->TAPR = value`
7. Set reload value: `TIMERx->TAILR = value`
8. Clear interrupt flag: `TIMERx->ICR = 0x01`
9. Enable timeout interrupt: `TIMERx->IMR |= 0x01`
10. Enable interrupt in NVIC: `NVIC->ISER[n] |= (1 << bit)`
11. Enable the timer: `TIMERx->CTL |= 0x01`
12. Implement the ISR (e.g., `TIMER1A_Handler()`)

#### GPTM Configuration Register Summary

| Register | Purpose | Typical Value |
|----------|---------|---------------|
| RCGCTIMER | Enable timer clock | Set bit for TIMERx |
| TIMERx->CFG | Select width (16-bit vs 32-bit) | 0x00 (32-bit) or 0x04 (16-bit) |
| TIMERx->TAMR | Set mode | 0x02 (periodic, count down) |
| TIMERx->TAILR | Set reload value | From desired period |
| TIMERx->TAPR | Prescaler (extends range in 16-bit mode) | 0-255 (16-bit only) |
| TIMERx->CTL | Enable timer | TAEN = 1 (bit 0) |
| TIMERx->IMR | Unmask interrupt | TATOIM = 1 (bit 0) |
| TIMERx->ICR | Clear interrupt flag (write-1-to-clear) | TATOCINT = 1 (bit 0) |