---
title: "Quadrature Encoder Interface (QEI)"
description: "Read a rotary encoder in hardware with the TM4C123's Quadrature Encoder Interface: track position and direction, measure speed with the velocity timer, and use an HW-040 knob as a PWM dimmer."
outline: deep
lastUpdated: true
datasheet: "/datasheets/tm4c123gh6pm.pdf#page=1305"
current: "/experiments/exp12-qei"
short: "QEI"
search: false
prev: false
next: false
head:
  - - meta
    - name: robots
      content: noindex
---

# Quadrature Encoder Interface (QEI)

This experiment reads a **rotary encoder**, the knob used for volume controls and menus, and the sensor on the shaft of most robot and industrial motors. The TM4C123 has a dedicated peripheral for it, the **Quadrature Encoder Interface (QEI)**, which decodes the encoder's two signals in hardware: it keeps the position, knows which way the shaft last turned, and measures how fast it's turning, all without CPU time. You will wire an HW-040 encoder module to QEI1, read its position and speed, and use it as a dimmer for the PWM output from [Experiment 8](/preview/exp8-pwm).

::: warning Reworked version: in testing
This is the reworked version of [Experiment 12](/experiments/exp12-qei), shared for TAs to test. In the new order it becomes Experiment 12. It has the same background, then one Activity built step by step and one Task. **Students: use the [current version](/experiments/exp12-qei)** until this one replaces it.
:::

::: warning In development
This experiment is new and isn't in the lab manual yet. If something on this page doesn't match what you see on the board, tell your instructor.
:::

## Objectives

- Explain how two quadrature signals encode both position and direction.
- Route the encoder's signals to QEI1 with `AFSEL` and `PCTL`, changing only the Port C pins you use, never the JTAG pins.
- Configure `QEICTL` and `QEIMAXPOS`, and read the position from `QEIPOS` and the direction from `QEISTAT`.
- Measure speed with the velocity timer (`QEILOAD`, `QEISPEED`) and its interrupt, and convert it to rpm.
- Use the knob as a dimmer: count whole clicks, change a PWM duty cycle by a step that depends on the speed, and switch the light with the knob's debounced button.

## Background

### What is a quadrature encoder?

An incremental encoder has two outputs, **A** and **B**. Turning the shaft makes each one produce a square wave, with the two shifted by a quarter of a period (90°, hence *quadrature*). The order in which they change gives the direction:

- **Forward:** A changes first, then B.
- **Reverse:** B changes first, then A.

Each full square wave on one output is one **pulse**, and it contains **four edges** (A rises, B rises, A falls, B falls). Counting every edge of both signals gives four times the resolution of counting pulses. The two signals are a 2-bit Gray code: only one of them changes at a time, so a reading is never ambiguous.

![Quadrature signals: turning forward, A leads B and the position counts up on every edge of A and B; turning in reverse, B leads A and the position counts down](/images/exp12/quadrature.svg)

Some encoders add a third output, **index** (I or Z), that pulses once per revolution to mark a home position. The HW-040 doesn't have one.

::: info Why not just use GPIO interrupts?
You could decode A and B in a GPIO interrupt handler (Experiment 6), but every edge would cost an interrupt, and a motor encoder can produce hundreds of thousands of edges per second. The QEI decodes up to a quarter of the system clock (12.5 MHz at 50 MHz) with no CPU time at all, and filters out noise on the way.
:::

### The HW-040 rotary encoder module

The HW-040 (sold also as KY-040) is a mechanical encoder on a small board with five pins:

| Pin | Signal | Connect to |
|:---|:---|:---|
| CLK | Output A | **PC5** (PhA1) |
| DT | Output B | **PC6** (PhB1) |
| SW | Push button, low when pressed | **PC4** (plain GPIO input; only in the Task) |
| + | Supply | **3.3 V** (not 5 V: the pull-ups pull to this pin) |
| GND | Ground | **GND** |

It gives **20 pulses per revolution**, one per click (detent) you feel as you turn it, so with all four edges counted, one turn is $20 \times 4 = 80$ counts, and one click is 4 counts ($4.5°$). The board has 10 kΩ pull-up resistors on CLK, DT, and SW.

::: warning Power it from 3.3 V
The HW-040 pulls its outputs up to whatever is on its **+** pin, so its signals are as high as its supply. PC4 to PC6 would survive 5 V as inputs, but PB0, PB1, PD4 and PD5 wouldn't, and it's a bad habit to drive a 3.3 V chip with 5 V. Use the LaunchPad's **3.3V** header pin.
:::

Because the contacts are mechanical, they **bounce**: one edge can arrive as a few quick transitions. For a quadrature signal this is mostly harmless, because a bounce on one signal while the other is steady counts +1, −1, +1 and settles on the right value. The QEI's input filter removes the shortest glitches.

### The QEI module

The TM4C123GH6PM has two QEI modules. Each has:

- A **position integrator** (`QEIPOS`) that counts up or down on every edge, wrapping at a maximum (`QEIMAXPOS`) or resetting on the index pulse.
- A **direction** bit (`QEISTAT`) for the last movement.
- A **velocity timer** (`QEILOAD`, `QEITIME`) and **velocity counter** (`QEICOUNT`, `QEISPEED`) that count edges in a fixed time window.
- A programmable **input filter**, and an interrupt for the index pulse, the end of each velocity window, a direction change, or a Gray-code error.

| Signal | QEI0 | QEI1 | PCTL value |
|:---|:---|:---|:---|
| PhA | PD6 or PF0 | **PC5** | 6 |
| PhB | PD7 or PF1 | **PC6** | 6 |
| IDX | PD3 or PF4 | PC4 | 6 |

This experiment uses **QEI1 on PC5 and PC6**. QEI0 would need PD7 or PF0, which are both locked pins (they can be the NMI input), and PF0, PF1 and PF4 are already the LaunchPad's button and LEDs.

::: danger PC0 to PC3 are the debugger's pins
Port C's lower four pins carry JTAG, the connection Keil uses to flash and debug the board. Always use read-modify-write on Port C (`|=`, `&= ~`, or masking `PCTL`) so only bits 4 to 6 change. Writing a whole value such as `GPIOC->AFSEL = 0x60` takes JTAG away, and the board then needs an unlock to be flashed again.
:::

#### Position modes

Bit `RESMODE` in `QEICTL` decides when the position goes back to 0:

- **`RESMODE = 0`:** the position counts between 0 and `MAXPOS`. Counting up past `MAXPOS` gives 0; counting down past 0 gives `MAXPOS`. With `MAXPOS = 79` and the HW-040, the position is the knob's angle within one turn.
- **`RESMODE = 1`:** the position resets to 0 on every index pulse. For encoders with an index output; not used here.

With `MAXPOS = 0xFFFFFFFF`, the counter simply wraps like any 32-bit number, and reading it as an `int32_t` gives a signed position: turning back from 0 gives −1, −2, and so on.

#### Measuring speed

With velocity capture on (`VELEN`), the QEI counts edges in a time window of `QEILOAD + 1` system clocks. At the end of each window, it copies the count into `QEISPEED`, clears the counter, and starts the next window. If the window is $f_\text{clock} / f_\text{window}$ clocks long:

$$
\text{QEILOAD} = \frac{f_{\text{clock}}}{f_{\text{window}}} - 1
\qquad
\text{rpm} = \frac{\text{SPEED} \times f_{\text{window}} \times 60}{\text{PPR} \times 4}
$$

**Worked example:** a 100 ms window at 50 MHz is `QEILOAD = 5,000,000 − 1`. If you turn the HW-040 at one revolution per second, the QEI counts 80 edges per second, so `SPEED = 8` in each 100 ms window, and $\text{rpm} = 8 \times 10 \times 60 / 80 = 60$.

`QEISPEED` has no sign: it counts edges in both directions. Use the `DIRECTION` bit in `QEISTAT` for the sign.

::: tip Speed resolution
With a 100 ms window, one edge more or less is $10 \times 60 / 80 = 7.5$ rpm, so the reading jumps in steps of 7.5. A longer window gives finer steps but updates less often. A motor encoder with 1000 pulses per revolution would give steps of 0.15 rpm with the same window.
:::

### QEI Registers

The QEI1 registers are reached as `QEI1->CTL`, `QEI1->POS`, and so on (base `0x4002D000`). Before any of them, enable the module's clock:

#### RCGCQEI — QEI Run Mode Clock Gating

**Register:** `SYSCTL->RCGCQEI` (`0x400FE644`)

Bit 0 enables QEI0 and bit 1 enables QEI1. As for the other peripherals, wait a few cycles after setting it before touching the QEI registers.

#### QEICTL — QEI Control

**Register:** `QEIx->CTL` (`Base + 0x000`)

**QEICTL Register — QEI Control**

| Bits | Field |
| --- | --- |
| 31–20 | reserved |
| 19–16 | FILTCNT |
| 15–14 | reserved |
| 13 | FILTEN |
| 12 | STALLEN |
| 11 | INVI |
| 10 | INVB |
| 9 | INVA |
| 8–6 | VELDIV |
| 5 | VELEN |
| 4 | RESMODE |
| 3 | CAPMODE |
| 2 | SIGMODE |
| 1 | SWAP |
| 0 | ENABLE |

| Bits | Field | Value used here | Meaning |
|:---|:---|:---|:---|
| 19:16 | FILTCNT | `0xF` | Filter samples the inputs every FILTCNT + 2 clocks (17) |
| 13 | FILTEN | 1 | Input filter on: an input must be stable for 3 samples |
| 8:6 | VELDIV | 0 | Divide edges by $2^\text{VELDIV}$ before the velocity counter (for fast motors) |
| 5 | VELEN | 1 | Velocity capture on |
| 4 | RESMODE | 0 | Wrap at `MAXPOS` (1 = reset on index) |
| 3 | CAPMODE | 1 | Count edges of both PhA and PhB (0 = PhA only: half the counts) |
| 2 | SIGMODE | 0 | Quadrature signals (1 = clock and direction) |
| 1 | SWAP | 0 | 1 swaps A and B: flips which way counts as forward |
| 0 | ENABLE | 1, last | Starts the module |

`INVA`, `INVB`, and `INVI` invert an input; `STALLEN` freezes the module while the debugger has the CPU stopped.

::: warning ENABLE can't be cleared
Once `ENABLE` is set, writing 0 to it does nothing; only a reset of the module (`SYSCTL->SRQEI`) or of the board turns it off. Write every other field first and set `ENABLE` last. If you change the configuration and re-flash, the board resets, so this only matters inside one program.
:::

#### QEISTAT — QEI Status

**Register:** `QEIx->STAT` (`Base + 0x004`)

**QEISTAT Register — QEI Status**

| Bits | Field |
| --- | --- |
| 31–2 | reserved |
| 1 | DIRECTION |
| 0 | ERROR |

- **Bit 1 (DIRECTION):** 0 = the last movement was forward (counting up), 1 = reverse.
- **Bit 0 (ERROR):** 1 = A and B changed at the same time, which a valid Gray code never does: noise, a loose wire, or an encoder spinning too fast for the filter.

#### QEIPOS, QEIMAXPOS, QEILOAD, and QEISPEED

**Registers:** `QEIx->POS` (`Base + 0x008`), `QEIx->MAXPOS` (`0x00C`), `QEIx->LOAD` (`0x010`), and `QEIx->SPEED` (`0x01C`)

Each is a single 32-bit value:

| Register | Access | Holds |
|:---|:---|:---|
| `POS` | Read/write | Current position. Write it **after** `ENABLE` to set a starting position. |
| `MAXPOS` | Read/write | Largest position before wrapping (`RESMODE = 0`) |
| `LOAD` | Read/write | Velocity window length minus one, in system clocks |
| `TIME` | Read only | Velocity timer's current value (counts down from `LOAD`) |
| `COUNT` | Read only | Edges so far in the current window |
| `SPEED` | Read only | Edges in the last complete window |

#### QEIINTEN, QEIRIS, and QEIISC — Interrupts

**Registers:** `QEIx->INTEN` (`Base + 0x020`), `QEIx->RIS` (`0x024`), and `QEIx->ISC` (`0x028`)

**QEIINTEN Register — QEI Interrupt Enable**

| Bits | Field |
| --- | --- |
| 31–4 | reserved |
| 3 | INTERROR |
| 2 | INTDIR |
| 1 | INTTIMER |
| 0 | INTINDEX |

The four bits are the same in all three registers: set a bit in `INTEN` to enable that interrupt, read `RIS` to see which events happened, and read `ISC` to see which *enabled* ones happened; writing 1 to a bit in `ISC` clears it, as with the timer interrupts in Experiment 7.

| Bit | Event |
|:---|:---|
| 3 | Gray-code error (`STAT.ERROR`) |
| 2 | Direction changed |
| 1 | Velocity window ended: a new `SPEED` value is ready |
| 0 | Index pulse |

In the NVIC, QEI1 is **interrupt 38** (`QEI1_IRQn`), and its handler is `QEI1_Handler`.

## Activity

Read the HW-040 knob three ways at once: its **position** within one turn (in counts, clicks, and degrees), the **direction** it last moved (green LED forward, red reverse), and its **speed** in rpm, measured ten times a second. QEI1 does all the counting; the program only reads registers.

::: info Setting up the project
This experiment runs on the TM4C123 LaunchPad. If you haven't made a project yet, follow the [Tiva LaunchPad Project](/setup/tiva-project) guide first, choosing **C**, and make sure its blinking-LED test works. All the code goes in that project's `main.c`.
:::

### Step 1: Wire the encoder

Connect the HW-040 to the LaunchPad with jumper wires:

| HW-040 | LaunchPad | Signal |
| --- | --- | --- |
| `CLK` | **PC5** | PhA1, encoder A |
| `DT` | **PC6** | PhB1, encoder B |
| `SW` | | Not used in the Activity |
| `+` | **3.3V** | Supply for the pull-ups |
| `GND` | **GND** | |

PC5 and PC6 are on the J4 header. Double-check you're on **PC**5 and **PC**6, not PB5 and PB6.

### Step 2: Add the definitions

Replace the contents of `main.c` with the include, the definitions, and the variables you'll watch in the debugger:

<<< @/snippets/preview/exp12/main.c#defines{c}

- **`PCTL_PC56_MASK`** covers the fields of PC5 (bits 23:20) and PC6 (bits 27:24) together, and **`PCTL_PC56_QEI`** puts 6 in both.
- The **`CTL_`** names are the [QEICTL](#qeictl-—-qei-control) bits this program sets. `RESMODE` and `SIGMODE` stay 0, so they need no names.
- **`EDGES`** is the number of counts in one turn: 20 pulses, 4 edges each.

### Step 3: Add the speed handler

Add the QEI1 interrupt handler after the definitions:

<<< @/snippets/preview/exp12/main.c#handler{c}

- It runs at the end of every 100 ms window, once `main` enables the interrupt. By then the QEI has copied the window's edge count into `SPEED`.
- It clears the interrupt first, as with the timer handlers in Experiment 7.
- `rpm` is the [speed formula](#measuring-speed) with $f_\text{window} = 10$. The multiplications come before the division so integer division doesn't round the result to 0. `SPEED` has no sign, so `DIRECTION` supplies it.

### Step 4: Write `main`

Add `main` at the end of the file:

<<< @/snippets/preview/exp12/main.c#main{c}

It does four things, in order:

1. **Hands PC5 and PC6 to QEI1.** Port C is only changed with `|=` and a masked `PCTL` write: a plain `=` would take PC0 to PC3 away from the debugger (see the warning in [The QEI module](#the-qei-module)). The pull-ups are a backup for encoders without their own. PF1 and PF3 become outputs for the direction LEDs.
2. **Configures and starts the position counter.** Everything goes into `CTL` **before** `ENABLE`, which can't be cleared afterwards, and `POS` is written **after** it, as the datasheet requires. `MAXPOS = 79` with `RESMODE = 0` keeps the position within one turn: forward past 79 gives 0, back past 0 gives 79.
3. **Starts the speed measurement.** `LOAD` sets the window to $f_\text{clock}/10$ clocks, minus one: 100 ms. The interrupt fires at the end of each window and runs the Step 3 handler.
4. **Reads the results** in the loop: the position from `POS`, and the direction from `DIRECTION` in [QEISTAT](#qeistat-—-qei-status), which holds the direction of the **last** edge, so the LED keeps its colour when the knob stops. The QEI counts every edge in hardware, so nothing is missed however long the loop takes.

Build, flash, and run, and add `position`, `clicks`, `angle`, and `rpm` to the Watch window.

<Checklist id="activity" title="Activity checklist">

- One click at a time: does `position` change by **4** per click, and `angle` by 18°?
- Does one full turn (20 clicks) bring `position` back to where it started?
- Is the LED green while turning forward, and red after one click back?
- At about one turn per second, does `rpm` read about 60, negative in reverse, and 0 soon after you stop?
- Can you say why the QEI never misses a click, however slow the main loop is?

</Checklist>

::: details More to try (optional)
| Try this | What you'll see |
| --- | --- |
| Set `WINDOW_HZ` to 4 | `rpm` moves in steps of 3 rpm instead of 7.5, but updates only four times a second ([Speed resolution](#measuring-speed)). |
| Set `MAXPOS` to `0xFFFFFFFF`, and add `volatile int32_t turns;` with `turns = (int32_t)QEI1->POS;` in the loop | `position` never wraps at 79, and `turns` goes negative when you turn back past 0: -1, -2, ... |
| Remove `CTL_CAPMODE` from the `CTL` write | Only PhA's edges count: 2 per click, 40 per turn, and the `rpm` reading halves. |
| Add `CTL_SWAP` (`1U << 1`) to the `CTL` write | Forward and reverse swap: the count goes down and the LED turns red when you turn clockwise. |
:::

::: details The complete program
<<< @/snippets/preview/exp12/main.c#full{c}
:::

::: details Why does the rpm jump around?
By hand you can turn the HW-040 at a few hundred rpm at most, so `SPEED` is a small number and each edge is worth 7.5 rpm (see [Speed resolution](#measuring-speed)). The reading also includes the bounce of the contacts.
:::

::: details Common problems
| Symptom | Cause and fix |
| --- | --- |
| `position` never changes | `RCGCQEI`, `AFSEL`, or `PCTL` is missing or wrong (6 for QEI), or the wires are on PB5/PB6 instead of **PC5/PC6**. Check `+` is on 3.3V. |
| `position` goes back and forth by one, never further | Only one channel is connected: check the DT wire to **PC6**. |
| Counts **down** when you turn clockwise | A and B are the other way round on your module. Swap the CLK and DT wires, or add the `SWAP` bit (bit 1) to the `CTL` write. |
| Changes by **2** per click | Your encoder has two clicks per pulse. Count the clicks in one full turn and how far `position` moves, and set `PPR` to the counts per turn divided by 4. |
| The debugger can't connect any more | A Port C register was written with `=` and took the JTAG pins. Use **LM Flash Programmer → Other Utilities → Debug Port Unlock** to recover the board, then fix the write. |
| `position` only goes up to 79 | That's `MAXPOS`: one turn, by design. For an unlimited count, use `MAXPOS = 0xFFFFFFFF`. |
| Writing `POS = 0` has no effect | It was written before `ENABLE`. Write it after. |
| `rpm` stays 0 | `NVIC_EnableIRQ(QEI1_IRQn)` is missing, `VELEN` isn't in the `CTL` write, or the handler's name is misspelled: it must be exactly `QEI1_Handler`. |
| Program stops in the handler and never leaves | The interrupt isn't cleared: write `INT_TIMER` to `ISC`. |
:::

## Task

Make the knob a proper **dimmer** for the blue LED: slow turns make fine changes, fast turns make big ones, and pressing the knob switches the light off and on.

- Each click changes the brightness by **1%** when the knob turns slower than 120 rpm, and by **5%** when faster. Forward is brighter.
- The brightness stays within **1 to 99%**. Turning past an end leaves it there, and turning back changes it again straight away.
- Pressing the knob switches the light **off**, and the next press back **on** at the brightness it had. While it's off, turning the knob does nothing.

**Wiring:** the HW-040 as in the Activity, plus **`SW` to PC4**. PF2 is the blue part of the RGB LED, so the direction LEDs are gone.

### The starter program

Start from this program. It compiles and runs: the LED glows at 25%, but the knob doesn't change it yet.

<<< @/snippets/preview/exp12/task.c{c}

::: details How the provided part works
The PWM is the [Experiment 8 Activity](/preview/exp8-pwm#activity)'s: Timer1A makes a 1 kHz signal on PF2, and `pwm_set()` sets its duty cycle, without the breathing. QEI1 is set up as in this Activity, with one change: `MAXPOS = 0xFFFFFFFF`, so the position never wraps within reach of a hand, and `QEI1_Handler` keeps `rpm` up to date every 100 ms. `light()` switches the LED: on, PF2 is driven by the PWM; off, `AFSEL` is cleared and PF2 is a plain output, low. The PWM keeps running, so the brightness is kept. `SysTick_Handler` runs every 1 ms and does the knob button's release half of the debounce, as for SW1 in the [Experiment 7 Activity](/preview/exp7-timers#activity). Don't change these.
:::

:::: details What to write: the five TODOs
Each TODO is highlighted in the starter, exactly where its code goes. Write them in this order: **3, 5, 4**, then **2, 1**. That way the knob works before you add the button.

::: details TODO 3: count whole clicks (the main loop)
- Read the position as a signed number, `(int32_t)QEI1->POS`, and subtract `last`: that's how many counts the knob has moved since the last whole click, negative in reverse. Signed subtraction handles the wrap from 0 to 0xFFFFFFFF for you.
- One click is 4 counts: divide by 4 into `steps`. Integer division drops the rest, so a knob halfway between two clicks gives 0.
- Move `last` on by the counts you used, `steps * 4`, not to the position now. The rest stays for next time, and no part of a click is lost.

:::

::: details TODO 5: change the brightness (the main loop)
- Only while `on` is 1, and only if `steps` isn't 0.
- Add `steps * size` to `level`. `steps` is negative in reverse, so the same line makes the LED dimmer.
- Clamp **`level`** itself to 1 to 99, then call `pwm_set(level)`. `pwm_set()` clamps too, but only what it sends to the timer: if `level` itself were left at, say, 140, turning back would need 41 clicks before anything changed.

:::

**Test now:** with `size` still always 1, each click forward makes the LED 1% brighter, and backward 1% dimmer. Add `level` to the Watch window: it stops at 99 and at 1, and turning back changes it at once.

::: details TODO 4: the speed-dependent step (the main loop)
- If `rpm` is above `FAST_RPM` **or** below `-FAST_RPM`, set `size` to `FAST_STEP`. `rpm` is negative in reverse, and turning fast backward should take big steps too.

:::

::: details TODO 2: the knob's button (in `main`)
- PC4 is set up like SW1 in Experiment 7, on Port C: input (`DIR` cleared), pull-up (`PUR`), digital (`DEN`), and a falling-edge interrupt: `IS`, `IBE`, and `IEV` cleared, `ICR` cleared, `IM` set, and `NVIC_EnableIRQ(GPIOC_IRQn)`.
- Port C holds the debugger's JTAG pins: use only `|=` and `&= ~` with `KNOB_SW`, never a plain `=` (see the warning in [The QEI module](#the-qei-module)).
- Don't set PC4's `AFSEL` bit: that would make it the QEI1 index input.
- It goes before `SysTick_Config`: `SysTick_Handler` reads PC4.

:::

::: details TODO 1: switch the light (`GPIOC_Handler`)
- First the debounce, as in the Experiment 7 Activity's `GPIOF_Handler`: clear the flag in `ICR`, mask `KNOB_SW` in `IM` so the bounces can't interrupt, and set `up_ms` to 0. `SysTick_Handler` unmasks it 20 ms after the release.
- Then flip `on` (`on = !on`) and call `light(on)`.
- While it's off, TODO 3 still moves `last` on, but TODO 5 doesn't change `level`: turning the knob while off does nothing, even after it's switched back on.

:::
::::

Build and flash, and add `level`, `rpm`, and `on` to the Watch window.

<Checklist id="task" title="Task checklist">

- Did you write the click counting and the clamp yourself?
- One click at a time: does `level` change by 1? Spinning fast: by 5 per click?
- Turned far forward, does `level` stop at 99, and one click back give 98 at once? The same at 1?
- Does a press switch the LED off, with turning doing nothing, and the next press bring it back at the same brightness?
- Is one press always exactly one switch?

</Checklist>
