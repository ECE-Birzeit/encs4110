---
title: "PWM with General-Purpose Timers"
short: "PWM with Timers"
description: "Generate pulse-width modulated signals in hardware with the TM4C123's General-Purpose Timers in PWM mode: set the frequency and duty cycle, route the output to a pin, and dim and fade the LaunchPad LEDs."
outline: deep
lastUpdated: true
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=704"
part: "Part 3: Advanced Peripherals"
---

# PWM with General-Purpose Timers

This experiment uses the General-Purpose Timer Modules (GPTM) from [Experiment 7](./exp7-timers) in a new mode: **PWM mode**, where the timer drives a pin high and low by itself, with no interrupt and no CPU time. You will set a PWM signal's frequency and duty cycle from two registers, route the timer's output to an LED pin, and change the duty cycle at run time to fade the LED. The same technique drives servo motors, sets DC motor speed, and dims lights.

::: warning In development
This experiment is new and isn't in the lab manual yet. If something on this page doesn't match what you see on the board, tell your instructor.
:::

## Objectives

- Explain what frequency and duty cycle mean for a PWM signal, and why PWM can control brightness or speed.
- Configure a GPTM timer in PWM mode and route its output to a pin with `AFSEL` and `PCTL`.
- Calculate `TAILR` and `TAMATCHR` for a given frequency and duty cycle.
- Use the prescaler as a timer extension to reach long PWM periods, such as the 20 ms of a servo signal.
- Change the duty cycle at run time from a SysTick interrupt.

## Background

### What is PWM?

A **pulse-width modulated** (PWM) signal is a square wave whose *on* time is adjustable. It's described by two numbers:

- **Frequency** $f$ (or period $T = 1/f$): how often the pattern repeats.
- **Duty cycle** $D$: the fraction of each period the signal is high.

$$
D = \frac{t_{\text{high}}}{T} \times 100\%
$$

If the signal switches faster than the load can follow, the load responds to the **average** voltage:

$$
V_{\text{avg}} = D \times V_{\text{high}}
$$

A 25% duty cycle on a 3.3 V pin averages about 0.83 V. An LED switched at 1 kHz flickers too fast for the eye to see, so it simply looks dimmer; a motor's inertia smooths the pulses into a lower speed. The pin itself is still purely digital: either fully on or fully off, which is why PWM wastes almost no power in the switch.

| Application | Typical frequency | What the duty cycle controls |
|:---|:---|:---|
| LED dimming | 500 Hz to 2 kHz | Brightness |
| DC motor (through a driver) | 1 kHz to 20 kHz | Speed |
| Hobby servo | 50 Hz (20 ms period) | Angle: a 1 ms pulse is one end, 2 ms the other |
| Buzzer / tone | 100 Hz to 5 kHz | Pitch comes from the frequency; 50% duty is loudest |

### Two ways to make PWM on the TM4C123

The TM4C123GH6PM has two kinds of hardware that can generate PWM:

- **General-Purpose Timers (GPTM)** in PWM mode: each 16/32-bit timer gives two PWM outputs (Timer A and Timer B), on the timer's **CCP** pins. This is what this experiment uses, because it builds directly on Experiment 7.
- **Two dedicated PWM modules** (M0PWM and M1PWM) with dead-band, fault inputs, and synchronized outputs, made for motor control and power converters. They are not covered here.

### How GPTM PWM mode works

In PWM mode the timer runs as a 16-bit **down-counter** in periodic mode, exactly like Experiment 7, but instead of interrupting at zero it drives its CCP pin:

1. When the counter **reloads** from `TAILR`, the pin goes **high**.
2. The counter counts down. When it **equals `TAMATCHR`**, the pin goes **low**.
3. The counter reaches 0, reloads, and the cycle repeats.

![GPTM PWM mode: the counter counts down from TAILR to 0; the pin goes high at each reload and low when the counter reaches TAMATCHR](/images/exp11/pwm-mode.svg)

So `TAILR` sets the period and `TAMATCHR` sets where the pin turns off. The pin is high while the counter travels from `TAILR` down to `TAMATCHR`, so a **smaller match value means a longer high time**:

$$
T = \frac{\text{TAILR} + 1}{f_{\text{clock}}}
\qquad
D = \frac{\text{TAILR} - \text{TAMATCHR}}{\text{TAILR} + 1}
$$

Rearranged for the values you write, with $N = \text{TAILR} + 1$ counts per period:

$$
\text{TAILR} = \frac{f_{\text{clock}}}{f_{\text{PWM}}} - 1
\qquad
\text{TAMATCHR} = \text{TAILR} - D \times N
$$

**Worked example:** 1 kHz at 50 MHz with 25% duty. $N = 50{,}000{,}000 / 1{,}000 = 50{,}000$, so `TAILR = 49,999`. The high time is $0.25 \times 50{,}000 = 12{,}500$ counts, so `TAMATCHR = 49,999 − 12,500 = 37,499`.

::: tip Keep the match inside the period
Choose `TAMATCHR` strictly between 0 and `TAILR`. A fully-on or fully-off output (0% or 100%) is an edge case for this hardware; if you need one, it's simpler to set the duty to 1% or 99%, or to switch the pin back to plain GPIO.
:::

#### The prescaler is a timer extension in PWM mode

In Experiment 7 the prescaler (`TAPR`) *divided* the clock. **In PWM mode it doesn't**: `TAPR` becomes the top 8 bits of a **24-bit** start value, and a second register, `TAPMR`, becomes the top 8 bits of the match value.

| Register | Bits it supplies | Meaning in PWM mode |
|:---|:---|:---|
| `TAILR` | 15:0 | Low 16 bits of the start (period) value |
| `TAPR` | 23:16 | High 8 bits of the start (period) value |
| `TAMATCHR` | 15:0 | Low 16 bits of the match value |
| `TAPMR` | 23:16 | High 8 bits of the match value |

With 16 bits alone, the longest period at 50 MHz is $65{,}536 / 50{,}000{,}000 \approx 1.31$ ms (763 Hz minimum). With the extension the counter has 24 bits, up to $2^{24} / 50{,}000{,}000 \approx 335$ ms. A 1 kHz signal fits in 16 bits; a 50 Hz servo signal needs the extension: its period is $50{,}000{,}000 / 50 = 1{,}000{,}000$ counts, a start value of $999{,}999$ = `0x0F423F`, so `TAPR = 0x0F` and `TAILR = 0x423F`.

### PWM Registers

Only the registers that are new or behave differently in PWM mode are listed here; `CFG`, `TAILR`, and `CTL`'s enable bit work as in [Experiment 7](./exp7-timers#gptm-configuration-registers).

#### GPTMTAMR — Timer A Mode Register

**Register:** `TIMERx->TAMR` — GPTM Timer A Mode (`Base + 0x004`)

**GPTMTAMR Register — Timer A Mode**

| Bits | Field |
| --- | --- |
| 31–12 | reserved |
| 11 | TAPLO |
| 10 | TAMRSU |
| 9 | TAPWMIE |
| 8 | TAILD |
| 7 | TASNAPS |
| 6 | TAWOT |
| 5 | TAMIE |
| 4 | TACDIR |
| 3 | TAAMS |
| 2 | TACMR |
| 1–0 | TAMR |

Three fields select PWM mode:

| Bits | Field | Value for PWM | Meaning |
|:---|:---|:---|:---|
| 3 | TAAMS | 1 | Alternate mode: PWM |
| 2 | TACMR | 0 | Edge-count mode (required for PWM) |
| 1:0 | TAMR | `0x2` | Periodic |

Together: `TIMERx->TAMR = 0x0A`. For Timer B, the same fields are in `TBMR` (`TIMERx->TBMR = 0x0A`).

#### GPTMTAMATCHR — Timer A Match Register

**Register:** `TIMERx->TAMATCHR` — GPTM Timer A Match (`Base + 0x030`)

**GPTMTAMATCHR Register — Timer A Match**

| Bits | Field |
| --- | --- |
| 31–16 | unused in 16-bit mode |
| 15–0 | TAMATCHR[15:0] |

The low 16 bits of the value at which the output goes low; PWM mode always runs the timer as a 16-bit half, so bits 31:16 play no part. Timer B uses `TBMATCHR` (`Base + 0x034`).

#### GPTMTAPR and GPTMTAPMR — Prescaler and Prescale Match

**Registers:** `TIMERx->TAPR` (`Base + 0x038`) and `TIMERx->TAPMR` (`Base + 0x040`)

**GPTMTAPR Register — Timer A Prescale** (in PWM mode: start value bits 23:16)

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7–0 | TAPSR |

**GPTMTAPMR Register — Timer A Prescale Match** (in PWM mode: match value bits 23:16)

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7–0 | TAPSMR |


In PWM mode, bits 7:0 of these registers extend the start value and the match value to 24 bits, as described above. Write 0 to both when the period fits in 16 bits. Timer B uses `TBPR` and `TBPMR`.

#### GPTMCTL — Output Level

**Register:** `TIMERx->CTL` — GPTM Control (`Base + 0x00C`)

**GPTMCTL Register — Timer Control**

| Bits | Field |
| --- | --- |
| 31–15 | reserved |
| 14 | TBPWML |
| 13 | TBOTE |
| 12 | reserved |
| 11–10 | TBEVENT |
| 9 | TBSTALL |
| 8 | TBEN |
| 7 | reserved |
| 6 | TAPWML |
| 5 | TAOTE |
| 4 | RTCEN |
| 3–2 | TAEVENT |
| 1 | TASTALL |
| 0 | TAEN |

Besides `TAEN` (bit 0), PWM mode uses:

- **Bit 6 (TAPWML)**: PWM output level. 0 = as described above (high, then low at the match); 1 = inverted (low, then high at the match).
- **Bit 14 (TBPWML)**: the same for Timer B.

The examples leave `TAPWML` at 0.

### Routing the Timer to a Pin

A pin can be plain GPIO (driven by `DATA`) or connected to one of several peripherals. Two GPIO registers select which:

#### GPIOAFSEL — Alternate Function Select

**Register:** `GPIOx->AFSEL` (`Base + 0x420`)

**GPIOAFSEL Register (Port x) — Alternate Function Select**

| Bits | Field |
| --- | --- |
| 31–8 | reserved |
| 7 | Px7 |
| 6 | Px6 |
| 5 | Px5 |
| 4 | Px4 |
| 3 | Px3 |
| 2 | Px2 |
| 1 | Px1 |
| 0 | Px0 |

Setting bit *n* hands pin *n* to a peripheral. Writing `DATA` no longer changes the pin.

#### GPIOPCTL — Port Control

**Register:** `GPIOx->PCTL` (`Base + 0x52C`)

**GPIOPCTL Register (Port x) — Port Control**

| Bits | Field |
| --- | --- |
| 31–28 | PMC7 |
| 27–24 | PMC6 |
| 23–20 | PMC5 |
| 19–16 | PMC4 |
| 15–12 | PMC3 |
| 11–8 | PMC2 |
| 7–4 | PMC1 |
| 3–0 | PMC0 |

`PMCn` selects the peripheral for pin *n*.

Each pin has a 4-bit field (pin *n* uses bits $4n+3$ to $4n$) that chooses **which** peripheral drives it. For the timer CCP pins on Port F, the value is **7**. Always change only your pin's field:

```c
GPIOF->PCTL = (GPIOF->PCTL & ~0x00000F00) | 0x00000700;   // PF2's field (bits 11:8) = 7
```

`DEN` must still be set, as for any digital pin. `DIR` doesn't matter: the peripheral controls the direction.

#### Which timer drives each LaunchPad LED

The three LEDs happen to sit on timer CCP pins, so every LED can be dimmed in hardware:

| Pin | LED | Timer output | PCTL field | Timer half |
|:---|:---|:---|:---|:---|
| PF1 | Red | T0CCP1 | bits 7:4 = 7 | Timer0 **B** |
| PF2 | Blue | T1CCP0 | bits 11:8 = 7 | Timer1 **A** |
| PF3 | Green | T1CCP1 | bits 15:12 = 7 | Timer1 **B** |

CCP**0** is always Timer **A** and CCP**1** Timer **B**. Because PWM mode needs the timer split into 16-bit halves (`CFG = 0x4`), Timer1 can drive the blue and green LEDs at the same time, each with its own duty cycle.

### Configuration Steps

To output PWM on a pin with Timer *x*, half A:

1. Enable the clocks: `SYSCTL->RCGCGPIO` for the port and `SYSCTL->RCGCTIMER` for the timer, then wait a few cycles.
2. Hand the pin to the timer: set its bit in `GPIOx->AFSEL`, write 7 into its `GPIOx->PCTL` field, and set its bit in `GPIOx->DEN`.
3. Disable the timer half: clear `TAEN` in `TIMERx->CTL`.
4. Select 16-bit mode: `TIMERx->CFG = 0x4`.
5. Select PWM, periodic: `TIMERx->TAMR = 0x0A`.
6. Write the period: `TIMERx->TAILR` (and `TIMERx->TAPR` for periods above 16 bits).
7. Write the match: `TIMERx->TAMATCHR` (and `TIMERx->TAPMR`).
8. Enable the timer half: set `TAEN` in `TIMERx->CTL`.

To change the duty cycle later, write a new value to `TAMATCHR`; there's no need to stop the timer.

| **Register** | **Purpose** | **Value in Example 1** |
|:---|:---|:---|
| `GPIOF->AFSEL` | Hand PF2 to a peripheral | bit 2 set |
| `GPIOF->PCTL` | Choose T1CCP0 for PF2 | bits 11:8 = 7 |
| `TIMER1->CFG` | 16-bit mode | `0x4` |
| `TIMER1->TAMR` | PWM, periodic | `0x0A` |
| `TIMER1->TAILR` | Period | `49,999` (1 kHz) |
| `TIMER1->TAMATCHR` | Where the pin goes low | `37,499` (25%) |
| `TIMER1->CTL` | Start | `TAEN = 1` |

*GPTM PWM Configuration Summary*

## Examples

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, and make sure its blinking-LED test works.
:::

**Target device:** TM4C123GH6PM

Both examples assume the 50 MHz system clock set by `SystemInit`, as in Experiment 7.

### Example 1: Dimming an LED

This example sets up Timer1A in PWM mode at 1 kHz with a 25% duty cycle on PF2, so the blue LED glows at a quarter of its brightness. Try changing `DUTY` to 5, 50, and 95 and re-flashing.

<<< @/snippets/pwm/example1.c

**Explanation:**
- `GPIOF->AFSEL` and `GPIOF->PCTL` disconnect PF2 from `GPIOF->DATA` and connect it to T1CCP0, Timer1A's output.
- `TIMER1->CFG = 0x4` splits Timer1 into two 16-bit halves; PWM mode only works on a half.
- `TIMER1->TAMR = 0x0A` sets `TAAMS = 1` (PWM), `TACMR = 0`, and `TAMR = 2` (periodic).
- `TIMER1->TAILR = 49,999` gives $50{,}000$ counts per period: 1 ms, or 1 kHz.
- `TIMER1->TAMATCHR = 37,499`: the pin is high for $49{,}999 - 37{,}499 = 12{,}500$ counts, 25% of the period.
- The `while (1)` loop is empty and no interrupt is used: the timer generates the signal on its own.

::: tip See it on the oscilloscope
Probe **PF2** on the LaunchPad header (ground on any GND pin). You should see a 1 kHz square wave that is high for 250 µs of every 1 ms. Changing `DUTY` changes the width of the high part, not the frequency.
:::

### Example 2: Fading an LED with SysTick

This example keeps the PWM setup from Example 1 and adds a SysTick interrupt every 10 ms. Each interrupt moves the duty cycle one percent up or down, so the blue LED "breathes": it fades in over about a second, then out again.

<<< @/snippets/pwm/example2.c

**Explanation:**
- `pwm_set()` converts a percentage into a match value and clamps it to 1–99%, keeping the match strictly inside the period.
- `SysTick_Config(SystemCoreClock / 100)` gives a 10 ms tick, as in Experiment 7.
- `SysTick_Handler()` only writes a new `TAMATCHR`. The timer keeps running; the new duty cycle takes effect from the next period.
- Two timers work together: Timer1A makes the fast 1 kHz signal, and SysTick changes it slowly.

::: info Why does the fade look uneven?
The eye's response to brightness isn't linear: the change from 1% to 10% looks much bigger than from 60% to 70%. Real dimmers correct for this with a lookup table (a "gamma curve"). You can try it in the lab work.
:::

## Lab work

### Lab work 1: RGB Color Mixing

Drive all three LEDs with PWM to mix colors: Timer0B on PF1 (red), Timer1A on PF2 (blue), and Timer1B on PF3 (green), all at 1 kHz.

**Requirements:**
- Configure the three outputs using the table in [Which timer drives each LaunchPad LED](#which-timer-drives-each-launchpad-led). Timer B uses `TBMR`, `TBILR`, `TBMATCHR`, and `TBEN` (bit 8 of `CTL`).
- Write a function `set_color(uint32_t r, uint32_t g, uint32_t b)` that takes three percentages.
- In the main loop, cycle through at least four colors (for example orange, purple, cyan, and white), holding each for one second. Use SysTick for the delay.

**Hint:** Timer0 and Timer1 each need their own `RCGCTIMER` bit, and Timer1's two halves share one `CFG` and one `CTL` register: configure both halves before enabling either.

### Lab work 2: Button-Controlled Brightness

Use the two LaunchPad buttons to set the blue LED's brightness: **SW1** (PF4) makes it brighter by 10%, **SW2** (PF0) makes it dimmer by 10%, limited to 10–90%.

**Requirements:**
- Use interrupt-driven input for both buttons, as in [Experiment 6](./exp6-gpio-interrupts). Remember that PF0 is locked and must be unlocked with `GPIOF->LOCK` and `GPIOF->CR` first.
- Debounce both buttons with a millisecond counter from SysTick, as in [Experiment 7, Lab work 1](./exp7-timers#lab-work-1-debouncing-a-push-button-with-systick).
- Add the current brightness to the Watch window and check it changes in steps of 10.

**Extra:** Replace the linear steps with a 10-entry lookup table that looks evenly spaced to the eye (for example 1, 2, 4, 7, 11, 18, 28, 43, 66, 99) and compare.

### Lab work 3: Color Knob with the ADC

Turn a potentiometer to pick the RGB LED's color: as the knob goes from one end to the other, the LED should move smoothly from red to green to blue and back toward red.

**Requirements:**
- Reuse the three PWM outputs and `set_color()` from Lab work 1.
- Read the potentiometer on **AIN0 (PE3)** with sample sequencer 3, by polling, as in [Experiment 9, Example 1](./exp9-adc#example-1-single-adc-channel-reading-polling-method). Take a new reading every 20 ms, timed with SysTick.
- Map the 12-bit reading (0 to 4095) to a color as described in the hint, and update the LED after every reading.
- Add the ADC reading and the three percentages to the Watch window, and check they change as you turn the knob.

**Wiring:** the potentiometer's two outer pins to **3.3 V** and **GND**, and its middle pin (the wiper) to **PE3**, as in Experiment 9.

::: details Hint: mapping the knob to a color
Split the knob's range into three equal parts. In each part, one color fades out while the next fades in:

| Reading | Red | Green | Blue |
|:---|:---|:---|:---|
| 0 to 1365 | 99% to 0% | 0% to 99% | 0% |
| 1366 to 2730 | 0% | 99% to 0% | 0% to 99% |
| 2731 to 4095 | 0% to 99% | 0% | 99% to 0% |

With `v` as the reading, `part = (v * 3) / 4096` gives which third you're in (0, 1 or 2), and `f = ((v * 3) % 4096) * 99 / 4096` how far through it you are (0 to 98). In part 0, red is `99 - f` and green is `f`; parts 1 and 2 follow the same pattern. Let `set_color()` clamp anything below 1% to 1%, as `pwm_set()` does.
:::

**Extra:** Replace the potentiometer with a **TMP36** temperature sensor on PE3, as in [Experiment 9, Lab work 1](./exp9-adc#lab-work-1-timer-based-temperature-measurement), and show the temperature as a color: blue at 20 °C, red at 35 °C. The TMP36 outputs 10 mV per °C with 500 mV at 0 °C, so $T = (V_{\text{mV}} - 500) / 10$. Pinch the sensor between two fingers to warm it and watch the color shift.
