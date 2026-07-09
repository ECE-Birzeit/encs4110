---
outline: deep
lastUpdated: true
prev:
  text: "5. GPIO Inputs and Interrupt Handling"
  link: /experiments/5-procedure
next:
  text: "7. Character LCD Display Interface"
  link: /experiments/7-procedure
---


# Hardware Timers and Timing Control

::: info Complete Lab Manual
For learning objectives and theoretical background, see the PDF manual:
 <a href="/manual/encs4110_6.pdf" target="_blank">Download Experiment 6 PDF</a>
:::

<Badge type="tip" text="TM4C123GH6PM" />


## Examples

### Example 1: Milliseconds Counter with SysTick Timer

<<< @/snippets/timers/systick.c

### Example 2: Maximum 16-bit Delay with GPTM

<<< @/snippets/timers/timers.c



## Tasks

### Task 1: Debouncing a Push Button with SysTick
Update your SysTick program so that the RED LED (PF1) toggles only when SW1 (PF4) is pressed and properly debounced.

> Use `SysTick_Config(SystemCoreClock/1000)` to generate a **1 ms** tick. Accumulate these ticks in a counter to measure longer intervals (e.g., 2000 ticks for 2 seconds).
* **Debouncing requirement:** Use the 1 ms SysTick counter to debounce SW1 (PF4) with a **150 ms** interval. When a valid press is detected (after debouncing), toggle the RED LED (PF1).

> [!Note] Hint
>  Declare global variables:
> ```c
> volatile uint32_t global_ms;
> volatile uint32_t last_ms;
> ```
> - Increment `global_ms` in the SysTick handler.
> - In the GPIOF ISR, check if `(global_ms - last_ms) >= 150` before toggling the LED and set `last_ms = global_ms;`.
### Task 2: Multiple Blinking LEDs with GPTM
Use three GPTM timers to blink each LED at different intervals:
- RED LED (PF1): Blink every 250 ms (use TIMER0A)
- BLUE LED (PF2): Blink every 500 ms (use TIMER1A)
- GREEN LED (PF3): Blink every 1000 ms (use TIMER2A)