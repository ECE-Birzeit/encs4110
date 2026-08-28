---
outline: deep
lastUpdated: true
search: false
---

# Matrix Keypad Interfacing — Polling vs. Interrupts

A standalone assignment that pulls together **Experiment 5** (GPIO polling and interrupts) and **Experiment 6** (SysTick and GPTM timers) into one build: a 4x4 matrix keypad, scanned continuously in the background, read two different ways.

**Target device:** TM4C123GH6PM.

## What Is a Matrix Keypad?

A 4x4 keypad packs 16 keys into just 8 pins — 4 **rows** and 4 **columns**. Each key sits at exactly one row/column crossing, and pressing it is the only thing that ever joins that row to that column.

Since there's no way to read 16 independent switches through 8 wires all at once, you *scan* instead: drive one row HIGH at a time while the rest stay LOW, and read the columns, which are pulled down so they idle LOW. If a column reads HIGH while row *N* happens to be the active one, the key at (row *N*, that column) is being pressed. Cycle through all four rows quickly enough and the whole thing feels instant.

![4x4 matrix keypad internal wiring diagram](/keypad-matrix-diagram.svg)

## Hardware Setup

| Signal | Pin | Direction |
|---|---|---|
| ROW1–ROW4 | PD0–PD3 | Output |
| COL1–COL4 | PC4–PC7 | Input, pull-down |
| RGB LED (onboard) | PF1 / PF2 / PF3 | Output (Red / Blue / Green) |

Rows idle LOW and are driven HIGH one at a time; columns rely on the internal pull-downs (`PDR`). The onboard RGB LED needs no special handling here — use it exactly as in earlier experiments. Before wiring anything up, check your specific keypad's ribbon pinout with a multimeter; cheap keypads aren't always labeled consistently.

## Skeleton

Here's everything you start with — pin definitions and the interfaces you're expected to fill in. No logic is implemented yet.

```c
#include "TM4C123.h"

// ---- Rows: PD0-PD3 (outputs) ----
#define ROW_MASK   0x0F                 // PD0-PD3

// ---- Columns: PC4-PC7 (inputs, pull-down) ----
#define COL1 0x10                       // PC4
#define COL2 0x20                       // PC5
#define COL3 0x40                       // PC6
#define COL4 0x80                       // PC7
#define COL_MASK (COL1 | COL2 | COL3 | COL4)

// ---- Onboard RGB LED: Port F ----
#define RED_LED   0x02                  // PF1
#define BLUE_LED  0x04                  // PF2
#define GREEN_LED 0x08                  // PF3
#define LED_MASK  (RED_LED | BLUE_LED | GREEN_LED)

extern const char keymap[4][4];         // TODO: fill in — matches the diagram

volatile uint32_t global_ms;            // TODO: free-running millisecond counter
volatile uint8_t  current_row;          // TODO: row currently driven HIGH (0-3)

void KeyPad_GPIO_Init(void);            // TODO: clocks + DIR/DEN/PDR for PC4-7, PD0-3, PF1-3

// Use the CMSIS SysTick_Config() function for the ~1 ms tick, e.g.
// SysTick_Config(SystemCoreClock / 1000) — no need to touch LOAD/VAL/CTRL directly.
void SysTick_Handler(void);             // TODO: advance current_row, increment global_ms

void KeyPad_HandleKey(char key);        // TODO: perform the action for `key` — see Key Actions below
```

`SysTick_Handler` and the row scan it drives are identical in both tasks below. The only thing that changes between them is how a keypress actually gets detected.

## Key Actions

Whichever detection method you end up using, every press should route through `KeyPad_HandleKey`. Here's what it needs to do:

| Key(s) | Action |
|---|---|
| `1` `2` `3` | Red / Green / Blue |
| `4` `5` `6` | Red+Green / Red+Blue / Green+Blue |
| `7` | Red+Green+Blue (white) |
| `0` | All off |
| `A` | Start/pause blink |
| `B` / `C` | Increase / decrease speed |
| `D` | Reset interval to 2 s |

The color keys are the simple part — they just set which LED bits are on, and when the blink is paused, that's exactly what shows: a solid color. Keys `A` through `D` are more interesting: they drive a **second, independent timer** — Timer1A, running in **32-bit mode** — that blinks whichever color is currently active. It starts at a 2-second interval, and `B`/`C` nudge that interval up or down by 0.25 s with every press — down to a floor of **0.25 s** (fastest), up to a ceiling of **10 s** (slowest).

```c
#define TICKS_PER_SEC   50000000UL          // 50 MHz system clock
#define BLINK_DEFAULT   (2 * TICKS_PER_SEC) // 2 s
#define BLINK_STEP      (TICKS_PER_SEC / 4) // 0.25 s
#define BLINK_MIN       (TICKS_PER_SEC / 4) // 0.25 s floor — fastest allowed
#define BLINK_MAX       (10 * TICKS_PER_SEC)// 10 s ceiling — slowest allowed

void Blink_Timer_Init(void);      // TODO: TIMER1A, 32-bit periodic mode, disabled at reset
void TIMER1A_Handler(void);       // TODO: toggle the active color at the current rate
```

> [!Warning] Floor and ceiling
> `B` and `C` must never push the interval past `BLINK_MIN` or `BLINK_MAX` — go faster than 0.25 s and it stops looking like a blink; go slower than 10 s and it stops looking like *blinking* at all. Enforce both *before* you touch `TAILR`, not after: on an unsigned value, subtracting past the floor doesn't give you a negative number, it wraps around to something enormous. So check `if (reload >= BLINK_MIN + BLINK_STEP)` before subtracting, and `if (reload + BLINK_STEP <= BLINK_MAX)` before adding — never subtract or add first and check the result.

## Task 1: Polling

This task detects presses entirely inside `main()`, using a plain `while(1)` loop — no interrupts involved.

**Requirements:**
- Every loop iteration, read the columns together with whichever row is currently active (`current_row`).
- When a column reads HIGH, decode the `(row, column)` pair using `keymap` and call `KeyPad_HandleKey`.

```c
void KeyPad_Poll(void);   // TODO: call every main-loop iteration
```

That's the whole base task. Don't worry yet about a key firing more than once while it's held down — that's covered in Extras, below.

## Task 2: Interrupts

This task swaps polling for GPIOC edge interrupts on PC4–PC7, while SysTick keeps scanning rows in the background exactly as it did in Task 1.

**Requirements:**
- Configure the columns with `IBE = 1` (interrupt on both edges) — see the note below for why.
- In the handler, snapshot `current_row`, then check the triggering column's current level: HIGH means a press — decode it and call `KeyPad_HandleKey`. LOW means a release; ignore it for now.

```c
void GPIOC_ColInterrupt_Init(void);   // TODO: edge interrupt config for PC4-7 + NVIC (IRQ 2)
void GPIOC_Handler(void);             // TODO: on a HIGH column, decode + call KeyPad_HandleKey
```

> [!Note] Why `IBE = 1`?
> Rows are never floating — the scan drives every row explicitly, HIGH on the active one and LOW on the rest. A column tied to a held key flips right along with the scan, not just on physical press or release, so you can't safely bet on one fixed edge direction (`IEV`) always meaning "pressed." `IBE = 1` interrupts on any change instead, leaving it to you to read the level and decide what actually happened.

## Extras (Optional)

Both base tasks above will visibly misbehave if you hold a key down: the action repeats every scan cycle — every few milliseconds — for as long as it's held. Fixing that is a natural next step.

**Debounce.** Use `global_ms` to require a minimum time between accepted presses, following the pattern from Experiment 6, Task 1.

**Task 1 — no repeat while held.** Restructure `KeyPad_Poll` as an explicit state machine with these states: `IDLE`, `DEBOUNCE`, `PRESSED`, `WAIT_RELEASE`. A column reading is only meaningful while its own row is active, so ask yourself: what breaks if your state machine checks a column at the wrong moment relative to `current_row`?

**Task 2 — no repeat while held.** This one's trickier. A held key's column pulses at the *scan* rate, not just on physical press and release, so a simple "latch on press, clear on any edge" design will still repeat every cycle. What do you need to remember *per column* to tell "the row cycled away" apart from "the key was actually released"?

**Keys `8`, `9`, `*`, `#`** are left open — propose and justify your own action for each.

## Verification Checklist

1. **Wiring** — confirm each of the 16 keys maps to the right `(row, column)` pair.
2. **Colors** — `1`–`7` show the right combination, and `0` clears them.
3. **Blink** — the default toggles roughly every 2 s; `A` starts and pauses it; `B`/`C` shift the interval by 0.25 s; `D` resets it back to 2 s.
4. **If you did the Extras** — holding a key for two or more seconds should fire its action exactly once.
