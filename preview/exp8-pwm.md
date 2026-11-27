---
title: "PWM with General-Purpose Timers"
description: "Generate pulse-width modulated signals in hardware with the TM4C123's General-Purpose Timers in PWM mode: set the frequency and duty cycle, route the output to a pin, and dim and fade the LaunchPad LEDs."
outline: deep
lastUpdated: true
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=704"
short: "PWM"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# PWM with General-Purpose Timers

This experiment uses the General-Purpose Timer Modules (GPTM) from [Experiment 7](/preview/exp7-timers) in a new mode: **PWM mode**, where the timer drives a pin high and low by itself, with no interrupt and no CPU time. You will set a PWM signal's frequency and duty cycle from two registers, route the timer's output to an LED pin, and change the duty cycle at run time to fade the LED. The same technique drives servo motors, sets DC motor speed, and dims lights.

::: warning New experiment: in testing
This experiment is part of the new lab manual, shared for TAs to test.
:::

::: warning In development
This experiment is new and isn't in the lab manual yet. If something on this page doesn't match what you see on the board, tell your instructor.
:::

## Objectives

- Explain what frequency and duty cycle mean for a PWM signal, and why PWM can control brightness or speed.
- Configure a GPTM timer half (Timer A or Timer B) in PWM mode and route its output to a pin with `AFSEL` and `PCTL`.
- Calculate `TAILR` and `TAMATCHR` for a given frequency and duty cycle.
- Use the prescaler as a timer extension (`TAPR` and `TAPMR`) to reach long PWM periods, such as the 20 ms of a servo signal.
- Change the duty cycle at run time, from a SysTick interrupt and from button presses.
- Drive several PWM outputs at once, including both halves of one timer, to mix colours on the RGB LED.

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

Only the registers that are new or behave differently in PWM mode are listed here; `CFG`, `TAILR`, and `CTL`'s enable bit work as in [Experiment 7](/preview/exp7-timers#gptm-configuration-registers).

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

The activity leaves `TAPWML` at 0.

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


## Activity

Make the blue LED **breathe**: fade in over about a second, then out again, forever. Timer1A generates a 1 kHz PWM signal on PF2 by itself, and a SysTick interrupt every 10 ms moves its duty cycle one percent up or down. You write `main.c` in three parts, top to bottom.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, and make sure its blinking-LED test works. All the code goes in that project's `main.c`.
:::

Nothing needs wiring: the blue LED is on the board. The code assumes the 50 MHz system clock set by `SystemInit`, as in Experiment 7.

### Step 1: Add the definitions

Replace the contents of `main.c` with the include and the definitions. Every pin, register bit, and number the program uses gets a name here, so the code below reads as what it does rather than as hex values.

<<< @/snippets/preview/exp8/main.c#defines{c}

- **`PCTL_PF2_MASK`** and **`PCTL_PF2_T1CCP0`** are PF2's 4-bit field in `PCTL` (bits 11:8, from the table in [Routing the Timer to a Pin](#routing-the-timer-to-a-pin)) and the value 7 shifted into it.
- **`TAMR_PWM`** is the three fields from the [TAMR table](#gptmtamr-—-timer-a-mode-register) combined: `TAAMS = 1`, `TACMR = 0`, `TAMR = 2`.
- **`PERIOD`** is $N = \text{TAILR} + 1$: 50,000 counts at 50 MHz is 1 ms, a 1 kHz signal.
- The bit masks use **`1U`**, an unsigned 1: a plain `1` is a signed `int`, and shifting or inverting signed values is where C's surprises live.

### Step 2: Add `pwm_set()` and the SysTick handler

Add these after the definitions:

<<< @/snippets/preview/exp8/main.c#functions{c}

- **`pwm_set()`** turns a percentage into a match value with the formula from [How GPTM PWM mode works](#how-gptm-pwm-mode-works). The pin is high from `TAILR` down to the match, so a **bigger** duty cycle means a **smaller** match value. The clamp keeps the match strictly inside the period.
- **`SysTick_Handler()`** runs every 10 ms once `main` starts SysTick. It moves `level` one percent and writes it with `pwm_set()`: 98 steps each way, about 2 seconds per breath. It never touches the timer's configuration, only its match value, which takes effect from the next period.
- `level` and `step` are `volatile` because the interrupt handler changes them, and **signed** (`int32_t`) because `step` is −1 half the time. `pwm_set()` takes a signed value for the same reason: a percentage that went below 0 clamps to 1%, where an unsigned one would wrap to about 4 billion and clamp to 99%.

### Step 3: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp8/main.c#main{c}

It does three things, in order:

1. **Hands PF2 to Timer1A.** Turn on both clocks and wait on the **peripheral ready** registers `PRGPIO` and `PRTIMER` instead of the `NOP`s of earlier experiments. Then `AFSEL` disconnects PF2 from `GPIOF->DATA`, and `PCTL` (with a mask, so the other pins keep their fields) picks T1CCP0 to drive it.
2. **Starts Timer1A in PWM mode,** with the settings from [How GPTM PWM mode works](#how-gptm-pwm-mode-works): stop, 16-bit, PWM mode, period, match, start. `TAPR` is cleared because a 1 kHz period fits in 16 bits.
3. **Starts SysTick** with an interrupt every 10 ms, as in Experiment 7, which begins the fade.

Build, flash, and run.

<Checklist id="activity" title="Activity checklist">

- Does the blue LED fade in and out by itself, one breath about every 2 seconds?
- In the Watch window, does `level` sweep between 1 and 99?
- With `SysTick_Config` commented out, does the LED glow steadily, brighter for a bigger `DUTY`, with no flicker?
- In the System Viewer, does GPIOF show `AFSEL = 0x04` and `PCTL = 0x00000700`?
- Can you say why a **bigger** duty cycle needs a **smaller** `TAMATCHR`?

</Checklist>

::: details More to try (optional)
| Try this | What you'll see |
| --- | --- |
| Comment out `SysTick_Config`, then set `DUTY` to 5, 50, and 95 | A steady glow whose brightness follows `DUTY`, with no flicker: 1 kHz is far too fast for the eye. |
| Probe **PF2** with an oscilloscope (ground on any GND pin), SysTick off, `DUTY` 25 | A 1 kHz square wave, high for 250 µs of every 1 ms. `DUTY` changes the width, not the frequency. With SysTick on, the high part grows and shrinks. |
| Set `PERIOD` to `1000000` (50 Hz, the 20 ms servo period), changing nothing else | No 50 Hz flicker: the timer keeps only the low 16 bits of `TAILR` and `TAMATCHR`, and 999,999 doesn't fit. |
| Now split both values with the [timer extension](#the-prescaler-is-a-timer-extension-in-pwm-mode): `TAPR`/`TAILR` in `main`, `TAPMR`/`TAMATCHR` in `pwm_set()`. Work out the four values for 25% by hand first | The LED **flickers**, most clearly out of the corner of your eye or when you move the board. Compare your values with **Peripherals → System Viewer → TIMER1**. Answer below. |
| Replace `pwm_set(level)` with a lookup in a table such as `1, 2, 4, 7, 11, 18, 28, 43, 66, 99` | A more even fade. The eye isn't linear: 1% to 10% looks a much bigger change than 60% to 70%, so real dimmers use a "gamma curve". |

The 50 Hz answer. In `main`: `TIMER1->TAPR = (PERIOD - 1) >> 16;` and `TIMER1->TAILR = (PERIOD - 1) & 0xFFFF;`. In `pwm_set()`, keep the match in a variable, `uint32_t match = (PERIOD - 1) - (PERIOD / 100) * (uint32_t)percent;`, then write `TIMER1->TAPMR = match >> 16;` and `TIMER1->TAMATCHR = match & 0xFFFF;`. The start value is 999,999 = `0x0F423F`, so `TAPR = 0x0F` and `TAILR = 0x423F`. For 25% the match is 999,999 − 250,000 = 749,999 = `0x0B71AF`: `TAPMR = 0x0B` and `TAMATCHR = 0x71AF`. PF2 is then high for 5 ms of every 20 ms; a servo would want a 1 to 2 ms pulse, a duty of 5 to 10%. Put `PERIOD` and the three lines back afterwards.
:::

::: details The complete program
<<< @/snippets/preview/exp8/main.c#full{c}
:::

::: details Common problems
| Symptom | Cause and fix |
| --- | --- |
| LED stays off | `TAEN` was never set, `RCGCTIMER` bit 1 is missing, or `PCTL` holds the wrong value (7 for T1CCP0). Check `TIMER1->CTL` and `GPIOF->PCTL` in the System Viewer. |
| LED fully on whatever `DUTY` is | `TAMR` is not `0x0A`, so the timer isn't in PWM mode, or `TAMATCHR` is outside 0 to `TAILR`. |
| Brightness right, but it flickers | The period is too long: check `PERIOD` and that `TAPR` is 0. |
| No 50 Hz flicker after changing `PERIOD` to 1,000,000 | The value doesn't fit in 16 bits: split it across `TAPR`/`TAILR` and `TAPMR`/`TAMATCHR`. |
| Brighter when `DUTY` goes down | `TAPWML` (bit 6 of `CTL`) is set and the output is inverted. Leave it at 0. |
| Program stops in `HardFault_Handler` | A Timer1 register was written before its clock was ready: keep the `PRTIMER` wait. |
| No fade, LED stays at 25% | `SysTick_Config` is missing or commented out, or the handler's name is misspelled: it must be exactly `SysTick_Handler`. |
:::

## Task

Build an **RGB colour picker** with the two buttons. All three LEDs are driven by PWM, so each colour has its own level, and together they mix: red and green make yellow, green and blue make cyan, red and blue make magenta, and all three make white.

- **SW1** selects the colour to change: red, green, blue, and round again.
- **SW2** adds 20% to the selected colour: 0, 20, 40, 60, 80, 100, then back to 0.

The levels are kept as round percentages, and the PWM code clamps them as `pwm_set()` does: 0 is shown as 1%, a faint glow, and 100 as 99%.

The three LEDs use three timer outputs, from the table in [Which timer drives each LaunchPad LED](#which-timer-drives-each-launchpad-led): **Timer0B on PF1** (red), **Timer1A on PF2** (blue, as in the Activity), and **Timer1B on PF3** (green). Timer B works exactly like Timer A, with its own registers: `TBMR`, `TBPR`, `TBILR`, and `TBMATCHR`, and its enable bit `TBEN` at bit 8 of `CTL`.

### The starter program

Start from this program. It compiles and runs: red and blue glow, but green and the buttons do nothing yet.

<<< @/snippets/preview/exp8/task.c{c}

::: details How the provided part works
The pins, red, and blue are set up for you. **Red's Timer0B** is the Activity's Timer1A setup with the Timer B registers, and **blue's Timer1A** is the Activity's. **`pwm_match()`** is the Activity's `pwm_set()` without the write: it clamps the percentage to 1 to 99 and returns the match value, so it works for any timer half. The debounce is the one from the [Experiment 7 Task](/preview/exp7-timers#task): each button has a **`ready`** flag, `GPIOF_Handler` calls `on_press()` once per press, and `SysTick_Handler`, every millisecond, makes a released button ready again. `on_press()` runs inside `GPIOF_Handler`, which is fine here: setting a colour is only three register writes, with no waiting. Don't change these.
:::

:::: details What to write: the five TODOs
Each TODO is highlighted in the starter, exactly where its code goes. Write them in order: **1, 2, 3** make green work, then **4** and **5** make the buttons work.

::: details TODO 1: hand PF3 to Timer1B (in `main`)
- Copy the two provided lines above it, for PF3: set `GREEN_LED` in `AFSEL`, then put 7 in PF3's `PCTL` field with `PCTL_PF3_MASK` and `PCTL_PF3_T1CCP1`.
- Change only PF3's field, with the mask, so PF1 and PF2 keep theirs. It's the same line as Step 3 of the Activity.

:::

::: details TODO 2: set up green, Timer1B (in `main`)
- Copy red's Timer0B block, for `TIMER1`: `TBMR`, `TBPR`, `TBILR`, and `TBMATCHR` (with `pwm_match(level[GREEN])`), then start it with `TBEN`.
- **Leave out the `CFG` line.** Timer1A and Timer1B share one `CFG`, and the provided code already split Timer1 into two 16-bit halves, while both were stopped.
- **Start it with `|=`:** `TIMER1->CTL |= CTL_TBEN`. The halves share one `CTL` too, and `TIMER1->CTL = CTL_TBEN` would clear `TAEN` and stop blue.

:::

::: details TODO 3: `set_color()`
- Three lines, one per colour: write `pwm_match()` of its percentage into its timer half's match register. Red is `TIMER0->TBMATCHR`, green `TIMER1->TBMATCHR`, and blue `TIMER1->TAMATCHR`.
- `pwm_match()` already clamps, so `set_color()` accepts any level.

:::

**Test now:** build and flash. All three colours start at 20%: the LED is a dim white. If it's purple, green is missing: check TODO 1 and TODO 2. Change the starting `level` values to `{ 100, 100, 0 }` and re-flash: yellow. Put them back.

::: details TODO 4: SW1 selects the next colour (`on_press()`)
- Step `selected` through `RED`, `GREEN`, `BLUE` (0, 1, 2), then back to `RED`. Either add one and go back to 0 after `BLUE`, or use `% 3`.
- SW1 only selects: the LED doesn't change. Add `selected` to the Watch window to see it.

:::

::: details TODO 5: SW2 steps the selected colour (`on_press()`)
- `level[selected]` is the selected colour's level. Add `STEP` to it, and if it went past 100, set it back to 0.
- Then show the new mix: `set_color(level[RED], level[GREEN], level[BLUE])`.
:::
::::

Build and flash, and add `selected` and `level` to the Watch window.

<Checklist id="task" title="Task checklist">

- Did you write the Timer1B setup and `set_color()` yourself?
- At the start, is the LED a dim white, with `selected` at 0 (red)?
- Six SW2 presses: does `level[0]` go 40, 60, 80, 100, 0, 20, one step per press, with the colour following?
- Does SW1 move `selected` one step per press (0, 1, 2, 0) without changing the LED, so SW2 then changes only that colour?
- Can you mix yellow, cyan, magenta, and a bright white?

</Checklist>
