---
outline: deep
lastUpdated: true
---

# Simon-Says Sequence Repeater — A Build Roadmap

A standalone assignment that pulls together **Experiment 5** (GPIO output), **Experiment 6** (GPIO interrupts), and **Experiment 7** (SysTick timing) into one build: the board plays a growing sequence of LED flashes, and the player has to repeat it back exactly.

**Target device:** TM4C123GH6PM.

## Introduction

### What Is a Sequence Repeater?

The board flashes a sequence of LEDs, one at a time, at a fixed pace. The player then repeats that exact sequence on a matching set of buttons — press 1 to match a flash on LED 1, and so on. Get the whole sequence right, and it grows by one more random step before playing again. Get any step wrong, and the round ends.

Button presses are handled entirely by GPIO interrupts here, not by polling the pins in the main loop — pressing a button fires an ISR immediately, rather than waiting to be noticed on the next pass through `while(1)`. The interesting engineering problem isn't the flashing or the interrupt configuration individually — it's holding a sequence in memory, replaying all of it every round (not just the newest step), and reliably telling "a real, distinct button press" apart from the same finger still resting on the button from a moment ago, entirely from inside a handler that has to stay short.

### Hardware Setup

| Signal | Pin | Direction |
|---|---|---|
| Button 1 – Button 4 | PC4 – PC7 | Input, pull-up |
| LED 1 – LED 4 (paired with buttons 1–4 by index) | PD0 – PD3 | Output |

Buttons idle HIGH (internal pull-up) and read LOW when pressed — the same convention as the onboard SW1/SW2 buttons from Experiment 6. Button *i* and LED *i* are meant to correspond — pressing Button 1 should match a flash on LED 1, and so on.

### Game Behavior

| Event | Result |
|---|---|
| Power-up | Sequence starts at length 1 — one random step is shown |
| Player repeats the whole sequence correctly | Sequence grows by one more random step, then plays again |
| Player presses the wrong button at any point | Round ends: all 4 LEDs flash together briefly, then a new game starts at length 1 |

There's no separate start button — the game begins automatically at power-up and restarts automatically after a game-over flash, so the hardware stays at exactly 4 buttons and 4 LEDs.

## Build Steps

You get two files to fill in — `simon.c`/`simon.h` hold the game logic, `main.c` just calls into them. Every function is stubbed out with `// TODO` comments and builds cleanly as-is; no logic is implemented yet.

::: code-group
<<< @/snippets/simon/main.c
<<< @/snippets/simon/simon.c
<<< @/snippets/simon/simon.h
:::

Fill it in through the five steps below, **in order** — don't start Step *N+1* until Step *N*'s check actually passes.

### Step 1: Configure the Ports

Fill in `Simon_GPIO_Init`: clocks for GPIOC and GPIOD, then `DIR`/`DEN`/`PUR` for PC4–PC7 (inputs, pull-up) and `DIR`/`DEN` for PD0–PD3 (outputs).

```c
void Simon_GPIO_Init(void)
{
    // TODO: enable the clocks for GPIOC and GPIOD
    // TODO: PC4-7 as inputs -- DIR cleared, DEN, PUR (pull-up)
    // TODO: PD0-3 as outputs -- DIR, DEN
}
```

**Verify on the board/debugger:** breakpoint right after `Simon_GPIO_Init()` returns; watch `GPIOC->DIR`/`DEN`/`PUR` (bits 4–7) and `GPIOD->DIR`/`DEN` (bits 0–3) and confirm they're set the way the comments describe. Hand-edit `GPIOD->DATA` bits 0–3 in the Watch window and confirm each LED lights. Then, with the debugger still running, physically hold down each of the 4 buttons in turn and confirm the matching bit in `GPIOC->DATA` reads LOW while held, HIGH while released.

### Step 2: Add the Millisecond Clock

Fill in `SysTick_Handler`: advance `global_ms` by one on every tick.

```c
void SysTick_Handler(void)
{
    // TODO: increment global_ms
}
```

**Verify:** add `global_ms` to the Watch window and confirm it climbs steadily at roughly 1000 counts per second, matching `SysTick_Config(SystemCoreClock / 1000)` in `main.c`.

### Step 3: Play Back the Sequence

Fill in `Simon_ShowSequence`: light LED *i* for a fixed, even duration with a visible gap before the next one, for each step in `sequence[0..sequence_len-1]`.

```c
void Simon_ShowSequence(void)
{
    // TODO -- Step 3: for i = 0 .. sequence_len-1, light LED (1 << sequence[i])
    //         on PD0-3 for a fixed duration with a gap between steps
}
```

**Verify:** before any game logic exists, manually set `sequence = {0, 2, 1}` and `sequence_len = 3` in the Watch window, then call `Simon_ShowSequence()` once (temporarily from `main`, before the loop). Confirm LEDs 1, 3, 2 flash in exactly that order, each for the same duration, with a clearly visible gap between them — not overlapping, not skipped.

### Step 4: Configure a Button Interrupt

Fill in `GPIOC_ButtonInterrupt_Init` (falling-edge interrupt on PC4–PC7 + NVIC) and `GPIOC_Handler` (debounce each button independently using `global_ms`, then record a press in `pressed_button`).

```c
void GPIOC_ButtonInterrupt_Init(void)
{
    // TODO -- Step 4: configure PC4-7 for a falling-edge interrupt + NVIC (GPIOC = IRQ 2)
}

void GPIOC_Handler(void)
{
    // TODO -- Step 4: for each of PC4-7 that triggered, clear its interrupt
    //         flag, and -- if at least DEBOUNCE_MS has passed since that
    //         button's own last accepted press (last_press_ms[i]) -- record
    //         it in pressed_button and update last_press_ms[i]
}
```

**Something to think about:** the debounce timestamp is tracked *per button* (`last_press_ms[4]`), not as one shared value. What would go wrong during a fast multi-button sequence if all 4 buttons shared a single debounce timestamp instead?

**Verify:** press each of the 4 buttons individually, several times each, and confirm `pressed_button` shows the correct index exactly once per physical press — holding a button down, or a bit of mechanical bounce, should not produce repeated values. Also confirm the corresponding bit in `GPIOC->MIS` clears immediately after your handler runs, and that the program never seems to "hang" on a press (a classic symptom of an interrupt flag that never gets cleared).

### Step 5: Wire Up the Game

Fill in `Simon_Run`: drive the full game using the pieces from Steps 3–4 — show the sequence, consume `pressed_button` in order (setting it back to `0xFF` once read), grow the sequence on a correct full repeat, end the round on a wrong press.

```c
void Simon_Run(void)
{
    // TODO -- Step 5: drive the game -- show the sequence, read back the
    //         player's presses in order, grow the sequence by one random
    //         step on a correct repeat, end the round on a wrong press --
    //         see the assignment page for the exact requirements
}
```

**Something to think about:** each new round needs a random step, and there's no dedicated random-number hardware on this board. Is there anything *already* changing on its own, whether or not anyone has pressed a button, that could stand in for one?

**Verify:** play at least 3 full rounds without a mistake, and confirm the *entire* sequence replays each round, not just the newest step. Then deliberately press a wrong button partway through a sequence and confirm all 4 LEDs flash together, followed by a fresh length-1 game.

## Extras (Optional)

### Score display

Show the current sequence length on an LCD or over UART, reusing the driver skill from another assignment.

### Speeding up

Make the playback pace get faster as the sequence grows, so later rounds are visibly harder than a simple memory test.

### Strict vs. forgiving mode

Real Simon-style games differ on this: does a wrong press end the game entirely (back to length 1), or just restart the *current* round's playback so the player can try that length again? Pick one, implement it, and be ready to justify the choice.

### Simultaneous presses

What should happen if two buttons' interrupts fire in the same instant — a mechanical bounce, or a genuine simultaneous press? Decide and justify your own behavior.

## Verification Checklist

1. **Ports** — buttons and LEDs are correctly wired and configured (Step 1).
2. **Timing** — `global_ms` increments steadily (Step 2).
3. **Playback** — a manually-set test sequence displays in the correct order with clear gaps between steps (Step 3).
4. **Input** — each button's interrupt reliably reports itself exactly once per physical press, with no hangs and no repeats (Step 4).
5. **Game logic** — across at least 3 played rounds, the sequence grows correctly on success and the game ends/restarts correctly on a wrong press (Step 5).
